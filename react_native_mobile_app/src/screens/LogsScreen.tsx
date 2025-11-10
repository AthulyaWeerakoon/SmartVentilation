"use client"

import type React from "react"
import { useState } from "react"
import { View, Text, StyleSheet, FlatList, RefreshControl } from "react-native"
import { colors } from "../theme/colors"
import type { RoomLog } from "../types"
import { LogCard } from "../components/LogCard"

interface LogsScreenProps {
  logs: RoomLog[]
}

export const LogsScreen: React.FC<LogsScreenProps> = ({ logs }) => {
  const [refreshing, setRefreshing] = useState(false)

  const handleRefresh = () => {
    setRefreshing(true)
    setTimeout(() => setRefreshing(false), 1000)
  }

  const renderLog = ({ item }: { item: RoomLog }) => <LogCard log={item} />

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.headerTitle}>All Logs</Text>
      </View>

      <FlatList
        data={logs}
        keyExtractor={(item, index) => `${item.timestamp}-${index}`}
        renderItem={renderLog}
        contentContainerStyle={styles.listContent}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={handleRefresh} tintColor={colors.primary} />}
        ListEmptyComponent={
          <View style={styles.emptyContainer}>
            <Text style={styles.emptyText}>No logs available</Text>
          </View>
        }
      />
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: colors.background,
  },
  header: {
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
  listContent: {
    padding: 20,
  },
  emptyContainer: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    paddingVertical: 40,
  },
  emptyText: {
    color: colors.textSecondary,
    fontSize: 16,
  },
})
