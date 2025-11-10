import AsyncStorage from "@react-native-async-storage/async-storage"

const DEVICE_ID_KEY = "roomlink_device_id"

export const saveDeviceId = async (deviceId: string): Promise<void> => {
  try {
    await AsyncStorage.setItem(DEVICE_ID_KEY, deviceId)
  } catch (error) {
    console.error("Failed to save device ID", error)
  }
}

export const getDeviceId = async (): Promise<string | null> => {
  try {
    return await AsyncStorage.getItem(DEVICE_ID_KEY)
  } catch (error) {
    console.error("Failed to get device ID", error)
    return null
  }
}

export const clearDeviceId = async (): Promise<void> => {
  try {
    await AsyncStorage.removeItem(DEVICE_ID_KEY)
  } catch (error) {
    console.error("Failed to clear device ID", error)
  }
}
