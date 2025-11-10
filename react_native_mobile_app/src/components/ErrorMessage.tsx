import type React from "react"
import { View, Text, StyleSheet } from "react-native"
import { colors } from "../theme/colors"

interface ErrorMessageProps {
  message: string
}

export const ErrorMessage: React.FC<ErrorMessageProps> = ({ message }) => {
  return (
    <View style={styles.container}>
      <Text style={styles.errorText}>{message}</Text>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    backgroundColor: colors.surface,
    borderLeftColor: colors.error,
    borderLeftWidth: 4,
    paddingHorizontal: 16,
    paddingVertical: 12,
    marginVertical: 12,
    borderRadius: 4,
  },
  errorText: {
    color: colors.error,
    fontSize: 14,
  },
})
