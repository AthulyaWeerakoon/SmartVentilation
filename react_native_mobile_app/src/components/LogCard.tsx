import type React from "react"
import { View, Text, StyleSheet } from "react-native"
import type { RoomLog } from "../types"
import { colors } from "../theme/colors"
import { formatTimestamp, formatValue } from "../utils/format"

interface LogCardProps {
  log: RoomLog
  isLatest?: boolean
}

export const LogCard: React.FC<LogCardProps> = ({ log, isLatest = false }) => {
  if (isLatest) {
    return (
      <View style={styles.latestContainer}>
        <Text style={styles.latestLabel}>Latest Reading</Text>
        <Text style={styles.timestamp}>{formatTimestamp(log.timestamp)}</Text>

        <View style={styles.gridContainer}>
          <View style={styles.gridItem}>
            <Text style={styles.label}>MQ2 (Air Quality)</Text>
            <Text style={styles.value}>{formatValue(log.mq2)}</Text>
          </View>
          <View style={styles.gridItem}>
            <Text style={styles.label}>MQ135 (Pollution)</Text>
            <Text style={styles.value}>{formatValue(log.mq135)}</Text>
          </View>
          <View style={styles.gridItem}>
            <Text style={styles.label}>Occupancy</Text>
            <Text style={styles.value}>{formatValue(log.occupancy)}</Text>
          </View>
          <View style={styles.gridItem}>
            <Text style={styles.label}>Circulation</Text>
            <Text style={styles.value}>{formatValue(log.circulation)}</Text>
          </View>
        </View>
      </View>
    )
  }

  return (
    <View style={styles.container}>
      <Text style={styles.timestamp}>{formatTimestamp(log.timestamp)}</Text>
      <View style={styles.compactGrid}>
        <View style={styles.compactItem}>
          <Text style={styles.compactLabel}>MQ2</Text>
          <Text style={styles.compactValue}>{formatValue(log.mq2)}</Text>
        </View>
        <View style={styles.compactItem}>
          <Text style={styles.compactLabel}>MQ135</Text>
          <Text style={styles.compactValue}>{formatValue(log.mq135)}</Text>
        </View>
        <View style={styles.compactItem}>
          <Text style={styles.compactLabel}>Occupancy</Text>
          <Text style={styles.compactValue}>{formatValue(log.occupancy)}</Text>
        </View>
        <View style={styles.compactItem}>
          <Text style={styles.compactLabel}>Circulation</Text>
          <Text style={styles.compactValue}>{formatValue(log.circulation)}</Text>
        </View>
      </View>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    backgroundColor: colors.surface,
    borderRadius: 8,
    padding: 16,
    marginBottom: 12,
    borderColor: colors.border,
    borderWidth: 1,
  },
  latestContainer: {
    backgroundColor: colors.surfaceLight,
    borderRadius: 12,
    padding: 24,
    marginBottom: 24,
    alignItems: "center",
    borderColor: colors.primaryLight,
    borderWidth: 1,
  },
  latestLabel: {
    color: colors.textTertiary,
    fontSize: 12,
    textTransform: "uppercase",
    letterSpacing: 1,
    marginBottom: 8,
  },
  timestamp: {
    color: colors.text,
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 24,
  },
  gridContainer: {
    width: "100%",
    flexDirection: "row",
    flexWrap: "wrap",
    justifyContent: "space-around",
  },
  gridItem: {
    width: "45%",
    alignItems: "center",
    marginVertical: 12,
  },
  label: {
    color: colors.textSecondary,
    fontSize: 12,
    marginBottom: 4,
  },
  value: {
    color: colors.primary,
    fontSize: 20,
    fontWeight: "bold",
  },
  compactGrid: {
    flexDirection: "row",
    flexWrap: "wrap",
    marginTop: 12,
  },
  compactItem: {
    width: "50%",
    paddingVertical: 8,
  },
  compactLabel: {
    color: colors.textSecondary,
    fontSize: 11,
    marginBottom: 2,
  },
  compactValue: {
    color: colors.primaryLight,
    fontSize: 14,
    fontWeight: "600",
  },
})
