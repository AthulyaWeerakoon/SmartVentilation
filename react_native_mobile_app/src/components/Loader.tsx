import type React from "react"
import { ActivityIndicator, View, Text, StyleSheet } from "react-native"
import { colors } from "../theme/colors"

interface LoaderProps {
  message?: string
}

export const Loader: React.FC<LoaderProps> = ({ message = "Loading..." }) => {
  return (
    <View style={styles.container}>
      <ActivityIndicator size="large" color={colors.primary} />
      <Text style={styles.message}>{message}</Text>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: colors.background,
  },
  message: {
    marginTop: 16,
    color: colors.textSecondary,
    fontSize: 14,
  },
})
