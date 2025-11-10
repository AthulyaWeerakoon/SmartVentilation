const express = require('express');
const basicAuth = require('basic-auth');
const mysql = require('mysql2/promise');
const fs = require('fs');
const http = require('http');
const https = require('https');

// ================== CONFIG ==================
const app = express();
app.use(express.json());

// ====== BASIC AUTH CONFIG ======
const DEVICE_AUTH = { user: 'device_user', pass: 'device_pass' };
const CLIENT_AUTH = { user: 'client_user', pass: 'client_pass' };

// ====== MYSQL CONFIG ======
// replace with your configuration details
const pool = mysql.createPool({
  host: process.env.MYSQL_DOMAIN || '<replace with the IP of your RDS>',
  port: process.env.MYSQL_PORT || 3306,
  user: process.env.MYSQL_USER || 'iot_logger',
  password: process.env.MYSQL_PASSWORD || 'iot_password',
  database: process.env.MYSQL_SCHEMA || 'iot_logs',
  waitForConnections: true,
  connectionLimit: 5,
  queueLimit: 0,
});

// ====== AUTH HELPER ======
function checkAuth(req, expected) {
  const creds = basicAuth(req);
  return creds && creds.name === expected.user && creds.pass === expected.pass;
}

// ================== ENDPOINTS ==================

// POST /add-log  → Add new device log
app.post('/add-log', async (req, res) => {
  if (!checkAuth(req, DEVICE_AUTH)) return res.status(401).send('Unauthorized');

  const { device_id, values } = req.body;
  if (!device_id || !values) return res.status(400).send('Missing parameters');

  try {
    const [timestamp, mq2, mq135, occupancy, circulation] = values
      .split(',')
      .map(v => v.trim());
    const conn = await pool.getConnection();
    await conn.execute(
      `INSERT INTO device_logs (device_id, timestamp, mq2, mq135, occupancy, circulation)
       VALUES (?, ?, ?, ?, ?, ?)`,
      [device_id, timestamp, parseFloat(mq2), parseFloat(mq135),
       parseFloat(occupancy), parseFloat(circulation)]
    );
    conn.release();
    res.status(200).send('Log added');
  } catch (err) {
    console.error('Add log error:', err);
    res.status(500).send('Database error');
  }
});

// GET /connect-id/otp?device_id=<uuid>
app.get('/connect-id/otp', async (req, res) => {
  if (!checkAuth(req, DEVICE_AUTH)) return res.status(401).send('Unauthorized');

  const { device_id } = req.query;
  if (!device_id) return res.status(400).send('Missing device_id');

  const otp = Math.floor(100000 + Math.random() * 900000);

  try {
    const conn = await pool.getConnection();
    await conn.execute(
      `INSERT INTO device_otp (device_id, otp, created_at)
       VALUES (?, ?, NOW())
       ON DUPLICATE KEY UPDATE otp = ?, created_at = NOW()`,
      [device_id, otp, otp]
    );
    conn.release();
    res.json({ otp });
  } catch (err) {
    console.error('OTP generation error:', err);
    res.status(500).send('Database error');
  }
});

// GET /connect-id/uid?otp=<number>
app.get('/connect-id/uid', async (req, res) => {
  if (!checkAuth(req, CLIENT_AUTH)) return res.status(401).send('Unauthorized');

  const { otp } = req.query;
  if (!otp) return res.status(400).send('Missing otp');

  try {
    const conn = await pool.getConnection();
    const [rows] = await conn.execute(
      `SELECT device_id FROM device_otp
       WHERE otp = ? AND created_at > NOW() - INTERVAL 10 MINUTE`,
      [otp]
    );
    conn.release();
    if (rows.length === 0) return res.status(404).send('OTP expired or invalid');
    res.json({ device_id: rows[0].device_id });
  } catch (err) {
    console.error('UID lookup error:', err);
    res.status(500).send('Database error');
  }
});

// GET /device-log?device_id=<uuid>
app.get('/device-log', async (req, res) => {
  if (!checkAuth(req, CLIENT_AUTH)) return res.status(401).send('Unauthorized');

  const { device_id } = req.query;
  if (!device_id) return res.status(400).send('Missing device_id');

  try {
    const conn = await pool.getConnection();
    const [rows] = await conn.execute(
      `SELECT timestamp, mq2, mq135, occupancy, circulation
       FROM device_logs
       WHERE device_id = ?
       ORDER BY timestamp DESC
       LIMIT 100`,
      [device_id]
    );
    conn.release();
    res.json(rows);
  } catch (err) {
    console.error('Device log error:', err);
    res.status(500).send('Database error');
  }
});

// CLEANUP JOB (expired OTPs)
setInterval(async () => {
  try {
    const conn = await pool.getConnection();
    await conn.execute(
      'DELETE FROM device_otp WHERE created_at < NOW() - INTERVAL 10 MINUTE'
    );
    conn.release();
    console.log('[Cleanup] Expired OTPs removed');
  } catch (err) {
    console.error('[Cleanup Error]', err);
  }
}, 10 * 60 * 1000);

// ================== HTTPS / HTTP SERVER ==================
const certificatePath = '/certs/certificate.pfx';
const passwordPath = '/certs/certificate_password.txt';
const hasCertificate = fs.existsSync(certificatePath);
const hasPassword = fs.existsSync(passwordPath);

let server;
const PORT = process.env.PORT || 3000;

if (hasCertificate && hasPassword) {
  const password = fs.readFileSync(passwordPath, 'utf8').trim();
  const options = {
    pfx: fs.readFileSync(certificatePath),
    passphrase: password,
  };
  server = https.createServer(options, app);
  console.log('Using HTTPS (certificate + password found)');
} else {
  server = http.createServer(app);
  console.log('No certificate found, using HTTP');
}

// Optional certificate mount if cert.pfx and password.txt for the certificate are available in /cert directory
server.listen(PORT, () => {
  const protocol = hasCertificate && hasPassword ? 'HTTPS' : 'HTTP';
  console.log(`Server running on ${protocol} port ${PORT}`);
});
