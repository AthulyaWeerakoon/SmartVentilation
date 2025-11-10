"use client"

import type React from "react"
import { useEffect, useState } from "react"
import { NavigationContainer } from "@react-navigation/native"
import { createStackNavigator } from "@react-navigation/stack"
import type { RoomLog, RootStackParamList } from "../types"
import { getDeviceId } from "../services/storage"
import { ConnectScreen } from "../screens/ConnectScreen"
import { RoomHomeScreen } from "../screens/RoomHomeScreen"
import { LogsScreen } from "../screens/LogsScreen"
import { colors } from "../theme/colors"
import { Loader } from "../components/Loader"

const Stack = createStackNavigator<RootStackParamList>()

export const RootNavigator: React.FC = () => {
  const [isConnected, setIsConnected] = useState<boolean | null>(null)
  const [currentLogs, setCurrentLogs] = useState<RoomLog[]>([])

  useEffect(() => {
    checkConnection()
  }, [])

  const checkConnection = async () => {
    const deviceId = await getDeviceId()
    setIsConnected(!!deviceId)
  }

  if (isConnected === null) {
    return <Loader />
  }

  return (
    <NavigationContainer>
      <Stack.Navigator
        screenOptions={{
          headerShown: false,
          cardStyle: { backgroundColor: colors.background },
           animation: 'slide_from_right' ,
        }}
      >
        {isConnected ? (
          <>
            <Stack.Screen name="RoomHome" options={{ animation: "slide_from_right" }}>
              {({ navigation }) => (
                <RoomHomeScreen
                  onDisconnect={() => setIsConnected(false)}
                  onViewAllLogs={(logs) => {
                    setCurrentLogs(logs)
                    navigation.navigate("Logs")
                  }}
                />
              )}
            </Stack.Screen>
            <Stack.Screen name="Logs" options={{ animation: 'slide_from_right' }}>
              {() => <LogsScreen logs={currentLogs} />}
            </Stack.Screen>
          </>
        ) : (
          <Stack.Screen name="Connect" options={{ animation: 'slide_from_right' }}>
            {() => <ConnectScreen onConnected={() => setIsConnected(true)} />}
          </Stack.Screen>
        )}
      </Stack.Navigator>
    </NavigationContainer>
  )
}
