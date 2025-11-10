export const formatTimestamp = (isoString: string): string => {
  const date = new Date(isoString)
  const options: Intl.DateTimeFormatOptions = {
    day: "numeric",
    month: "short",
    year: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  }
  return date.toLocaleDateString("en-US", options)
}

export const formatValue = (value: number, decimals = 2): string => {
  return value.toFixed(decimals)
}
