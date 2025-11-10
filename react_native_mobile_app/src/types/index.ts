export interface RoomLog {
  timestamp: string
  mq2: number
  mq135: number
  occupancy: number
  circulation: number
}

export interface ConnectResponse {
  device_id: string
}

export type RootStackParamList = {
  Connect: undefined
  RoomHome: undefined
  Logs: undefined
}
