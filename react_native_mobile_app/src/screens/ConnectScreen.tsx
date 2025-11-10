"use client"

import type React from "react"
import { useState } from "react"
import { View, Text, StyleSheet, TextInput, TouchableOpacity, KeyboardAvoidingView, Platform } from "react-native"
import { colors } from "../theme/colors"
import { connectToRoom } from "../services/api"
import { saveDeviceId } from "../services/storage"
import { ErrorMessage } from "../components/ErrorMessage"
import { Loader } from "../components/Loader"

interface ConnectScreenProps {
  onConnected: () => void
}

export const ConnectScreen: React.FC<ConnectScreenProps> = ({ onConnected }) => {
  const [otp, setOtp] = useState("")
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const handleConnect = async () => {
    if (!otp.trim()) {
      setError("Please enter an OTP")
      return
    }

    setLoading(true)
    setError(null)

    try {
      const deviceId = await connectToRoom(otp)
      await saveDeviceId(deviceId)
      onConnected()
    } catch (err) {
      const errorMessage = err instanceof Error ? err.message : "Connection failed"
      setError(errorMessage)
    } finally {
      setLoading(false)
    }
  }

  if (loading) {
    return <Loader message="Connecting to room..." />
  }

  return (
    <KeyboardAvoidingView behavior={Platform.OS === "ios" ? "padding" : "height"} style={styles.container}>
      <View style={styles.content}>
        <View style={styles.header}>
          <Text style={styles.title}>Connect to a Room</Text>
          <Text style={styles.subtitle}>Enter the one-time PIN provided by your IoT device</Text>
        </View>

        {error && <ErrorMessage message={error} />}

        <View style={styles.inputContainer}>
          <TextInput
            style={styles.input}
            placeholder="Enter OTP"
            placeholderTextColor={colors.textTertiary}
            keyboardType="number-pad"
            value={otp}
            onChangeText={setOtp}
            maxLength={6}
            editable={!loading}
          />
        </View>

        <TouchableOpacity
          style={[styles.button, loading && styles.buttonDisabled]}
          onPress={handleConnect}
          disabled={loading}
        >
          <Text style={styles.buttonText}>Connect</Text>
        </TouchableOpacity>

        <Text style={styles.helpText}>OTP is typically a 6-digit code displayed on your device</Text>
      </View>
    </KeyboardAvoidingView>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.background,
  },
  content: {
    flex: 1,
    justifyContent: "center",
    paddingHorizontal: 24,
  },
  header: {
    marginBottom: 48,
    alignItems: "center",
  },
  title: {
    color: colors.text,
    fontSize: 32,
    fontWeight: "bold",
    marginBottom: 8,
    textAlign: "center",
  },
  subtitle: {
    color: colors.textSecondary,
    fontSize: 14,
    textAlign: "center",
    lineHeight: 20,
  },
  inputContainer: {
    marginBottom: 24,
  },
  input: {
    backgroundColor: colors.surface,
    borderColor: colors.border,
    borderWidth: 1,
    borderRadius: 8,
    paddingHorizontal: 16,
    paddingVertical: 16,
    color: colors.text,
    fontSize: 18,
    textAlign: "center",
    letterSpacing: 4,
  },
  button: {
    backgroundColor: colors.primary,
    borderRadius: 8,
    paddingVertical: 16,
    alignItems: "center",
    marginBottom: 24,
  },
  buttonDisabled: {
    opacity: 0.6,
  },
  buttonText: {
    color: colors.text,
    fontSize: 16,
    fontWeight: "600",
  },
  helpText: {
    color: colors.textTertiary,
    fontSize: 12,
    textAlign: "center",
    lineHeight: 18,
  },
})
