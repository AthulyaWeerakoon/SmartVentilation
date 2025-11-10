import axios, { type AxiosError } from "axios"
import type { RoomLog, ConnectResponse } from "../types"

const BASE_URL = "https://room-server-mini-jni2fwr6g-athulyaweerakoons-projects.vercel.app"
const BASIC_AUTH = {
  username: "client_user",
  password: "client_pass",
}

const apiClient = axios.create({
  baseURL: BASE_URL,
  auth: BASIC_AUTH,
  timeout: 10000,
})

export const connectToRoom = async (otp: string): Promise<string> => {
  try {
    const response = await apiClient.get<ConnectResponse>(`/connect-id/uid?otp=${otp}`)
    return response.data.device_id
  } catch (error) {
    const axiosError = error as AxiosError
    if (axiosError.response?.status === 404) {
      throw new Error("OTP expired or invalid")
    }
    if (axiosError.response?.status === 401) {
      throw new Error("Authentication failed")
    }
    throw new Error("Failed to connect to room")
  }
}

export const fetchRoomLogs = async (deviceId: string): Promise<RoomLog[]> => {
  try {
    const response = await apiClient.get<RoomLog[]>(`/device-log?device_id=${deviceId}`)
    return response.data
  } catch (error) {
    const axiosError = error as AxiosError
    if (axiosError.response?.status === 401) {
      throw new Error("Authentication failed")
    }
    if (axiosError.response?.status === 400) {
      throw new Error("Missing device ID")
    }
    throw new Error("Failed to fetch room logs")
  }
}
