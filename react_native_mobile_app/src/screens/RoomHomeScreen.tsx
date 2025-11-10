"use client"

import type React from "react"
import { useState, useEffect } from "react"
import { View, Text, StyleSheet, TouchableOpacity, ScrollView, RefreshControl } from "react-native"
import { Ionicons } from "@expo/vector-icons"
import { colors } from "../theme/colors"
import type { RoomLog } from "../types"
import { fetchRoomLogs } from "../services/api"
import { getDeviceId, clearDeviceId } from "../services/storage"
import { LogCard } from "../components/LogCard"
import { Loader } from "../components/Loader"
import { ErrorMessage } from "../components/ErrorMessage"

interface RoomHomeScreenProps {
  onDisconnect: () => void
  onViewAllLogs: (logs: RoomLog[]) => void
}

export const RoomHomeScreen: React.FC<RoomHomeScreenProps> = ({ onDisconnect, onViewAllLogs }) => {
  const [latestLog, setLatestLog] = useState<RoomLog | null>(null)
  const [logs, setLogs] = useState<RoomLog[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)
  const [refreshing, setRefreshing] = useState(false)

  useEffect(() => {
    loadLogs()
  }, [])

  const loadLogs = async () => {
    try {
      setError(null)
      const deviceId = await getDeviceId()
      if (!deviceId) {
        setError("Device ID not found")
        return
      }

      const fetchedLogs = await fetchRoomLogs(deviceId)
      if (fetchedLogs.length > 0) {
        setLatestLog(fetchedLogs[0])
        setLogs(fetchedLogs)
      } else {
        setError("No logs available")
      }
    } catch (err) {
      const errorMessage = err instanceof Error ? err.message : "Failed to load logs"
      setError(errorMessage)
    } finally {
      setLoading(false)
      setRefreshing(false)
    }
  }

  const handleRefresh = () => {
    setRefreshing(true)
    loadLogs()
  }

  const handleDisconnect = async () => {
    await clearDeviceId()
    onDisconnect()
  }

  const handleViewAllLogs = () => {
    onViewAllLogs(logs)
  }

  if (loading) {
    return <Loader message="Loading room data..." />
  }

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.headerTitle}>Room Status</Text>
        <TouchableOpacity onPress={handleDisconnect} style={styles.disconnectButton}>
          <Ionicons name="log-out-outline" size={24} color={colors.error} />
        </TouchableOpacity>
      </View>

      <ScrollView
        contentContainerStyle={styles.scrollContent}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={handleRefresh} tintColor={colors.primary} />}
      >
        {error && <ErrorMessage message={error} />}

        {latestLog && (
          <View style={styles.content}>
            <LogCard log={latestLog} isLatest={true} />

            <TouchableOpacity style={styles.viewAllButton} onPress={handleViewAllLogs}>
              <Text style={styles.viewAllText}>View All Logs</Text>
              <Ionicons name="chevron-forward" size={20} color={colors.primary} />
            </TouchableOpacity>
          </View>
        )}
      </ScrollView>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.background,
  },
  header: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    paddingHorizontal: 20,
    paddingTop: 16,
    paddingBottom: 16,
    borderBottomColor: colors.border,
    borderBottomWidth: 1,
  },
  headerTitle: {
    color: colors.text,
    fontSize: 24,
    fontWeight: "bold",
  },
  disconnectButton: {
    padding: 8,
  },
  scrollContent: {
    padding: 20,
  },
  content: {
    flex: 1,
  },
  viewAllButton: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    backgroundColor: colors.surface,
    borderRadius: 8,
    paddingVertical: 14,
    marginTop: 16,
    borderColor: colors.border,
    borderWidth: 1,
  },
  viewAllText: {
    color: colors.primary,
    fontSize: 16,
    fontWeight: "600",
    marginRight: 8,
  },
})
