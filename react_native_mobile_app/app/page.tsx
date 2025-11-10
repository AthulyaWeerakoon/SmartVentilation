export default function Page() {
  return (
    <div style={styles.container}>
      <div style={styles.content}>
        <h1 style={styles.title}>RoomLink - React Native App</h1>
        <p style={styles.subtitle}>IoT Device Connection & Sensor Data Logger</p>

        <div style={styles.section}>
          <h2 style={styles.heading}>Getting Started</h2>
          <p style={styles.text}>This is a React Native app built with Expo and TypeScript. To run this app locally:</p>

          <ol style={styles.list}>
            <li style={styles.listItem}>Download or clone the project from GitHub</li>
            <li style={styles.listItem}>
              Install dependencies: <code style={styles.code}>npm install</code> or{" "}
              <code style={styles.code}>yarn install</code>
            </li>
            <li style={styles.listItem}>
              Start the development server: <code style={styles.code}>expo start</code>
            </li>
            <li style={styles.listItem}>Scan the QR code with Expo Go app on iOS or Android</li>
          </ol>
        </div>

        <div style={styles.section}>
          <h2 style={styles.heading}>Configuration</h2>
          <p style={styles.text}>
            Before running the app, update the API endpoint in <code style={styles.code}>src/services/api.ts</code>:
          </p>
          <div style={styles.codeBlock}>
            <code>const BASE_URL = 'https://your-api-endpoint.com';</code>
          </div>
        </div>

        <div style={styles.section}>
          <h2 style={styles.heading}>Features</h2>
          <ul style={styles.featureList}>
            <li style={styles.featureItem}>OTP-based device connection</li>
            <li style={styles.featureItem}>Real-time sensor data display</li>
            <li style={styles.featureItem}>Persistent device storage</li>
            <li style={styles.featureItem}>Pull-to-refresh functionality</li>
            <li style={styles.featureItem}>Minimal dark UI with Ionicons</li>
            <li style={styles.featureItem}>Error handling and loading states</li>
          </ul>
        </div>

        <div style={styles.section}>
          <h2 style={styles.heading}>Project Structure</h2>
          <div style={styles.tree}>
            <div>src/</div>
            <div style={styles.indent}>├── components/ - Reusable UI components</div>
            <div style={styles.indent}>├── screens/ - Navigation screens</div>
            <div style={styles.indent}>├── services/ - API and storage services</div>
            <div style={styles.indent}>├── navigation/ - Navigation setup</div>
            <div style={styles.indent}>├── theme/ - Color system</div>
            <div style={styles.indent}>├── types/ - TypeScript types</div>
            <div style={styles.indent}>└── utils/ - Helper functions</div>
            <div>App.tsx - Main entry point</div>
            <div>app.json - Expo configuration</div>
          </div>
        </div>

        <div style={styles.footer}>
          <p>
            Ready to deploy? Use <code style={styles.code}>expo publish</code> or build with EAS.
          </p>
        </div>
      </div>
    </div>
  )
}

const styles = {
  container: {
    minHeight: "100vh",
    backgroundColor: "#121212",
    color: "#FFFFFF",
    fontFamily: "system-ui, -apple-system, sans-serif",
    padding: "40px 20px",
  },
  content: {
    maxWidth: "800px",
    margin: "0 auto",
  },
  title: {
    fontSize: "48px",
    fontWeight: "bold",
    marginBottom: "8px",
    color: "#FFFFFF",
  },
  subtitle: {
    fontSize: "18px",
    color: "#B0B0B0",
    marginBottom: "48px",
  },
  section: {
    marginBottom: "40px",
    paddingBottom: "24px",
    borderBottom: "1px solid #333333",
  },
  heading: {
    fontSize: "24px",
    fontWeight: "600",
    marginBottom: "16px",
    color: "#4CAF50",
  },
  text: {
    fontSize: "16px",
    lineHeight: "1.6",
    color: "#B0B0B0",
    marginBottom: "12px",
  },
  list: {
    marginLeft: "24px",
    marginTop: "12px",
  },
  listItem: {
    marginBottom: "8px",
    color: "#B0B0B0",
    fontSize: "16px",
  },
  code: {
    backgroundColor: "#1E1E1E",
    padding: "2px 6px",
    borderRadius: "4px",
    fontFamily: "monospace",
    color: "#4CAF50",
  },
  codeBlock: {
    backgroundColor: "#1E1E1E",
    padding: "16px",
    borderRadius: "8px",
    marginTop: "12px",
    fontFamily: "monospace",
    fontSize: "14px",
    color: "#4CAF50",
    overflow: "auto",
  },
  featureList: {
    marginLeft: "24px",
    marginTop: "12px",
  },
  featureItem: {
    marginBottom: "8px",
    color: "#B0B0B0",
    fontSize: "16px",
  },
  tree: {
    backgroundColor: "#1E1E1E",
    padding: "16px",
    borderRadius: "8px",
    fontFamily: "monospace",
    fontSize: "14px",
    color: "#B0B0B0",
    marginTop: "12px",
  },
  indent: {
    marginLeft: "16px",
  },
  footer: {
    marginTop: "40px",
    paddingTop: "24px",
    borderTop: "1px solid #333333",
    textAlign: "center" as const,
  },
} as const
