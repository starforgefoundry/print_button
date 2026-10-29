import { z } from 'zod'

export const INDICATOR_STATES = ['setup', 'unconfigured', 'offline', 'error', 'busy', 'plateReady', 'idle'] as const

export const deviceSchema = z.object({
  hostname: z.string(),
  ip: z.string(),
  mac: z.string(),
  firmwareVersion: z.string(),
  setupPortal: z.boolean(),
  wifiSsid: z.string(),
  wifiConnected: z.boolean(),
  rssi: z.number(),
  bambuddyUrl: z.string(),
  hasApiKey: z.boolean(),
  printerId: z.number(),
  printerName: z.string(),
  indicator: z.enum(INDICATOR_STATES),
  printerState: z.string(),
  error: z.string(),
})

export const printerSchema = z.object({
  id: z.number(),
  name: z.string(),
  model: z.string().nullable().optional(),
})

export const networkSchema = z.object({
  ssid: z.string(),
  rssi: z.number(),
  secure: z.boolean(),
})

export const peerSchema = z.object({
  hostname: z.string(),
  ip: z.string(),
  printerName: z.string(),
})

const okSchema = z.object({ ok: z.literal(true) })
const errorSchema = z.object({ error: z.string() })

export type Device = z.infer<typeof deviceSchema>
export type IndicatorState = Device['indicator']
export type Printer = z.infer<typeof printerSchema>
export type WifiNetwork = z.infer<typeof networkSchema>
export type Peer = z.infer<typeof peerSchema>

export interface BambuddyConnection {
  bambuddyUrl: string
  apiKey: string
}

export interface PrinterSelection extends BambuddyConnection {
  printerId: number
  printerName: string
}

export interface WifiCredentials {
  ssid: string
  password: string
}

async function request<T>(path: string, schema: z.ZodType<T>, init?: RequestInit): Promise<T> {
  const response = await fetch(path, {
    ...init,
    headers: init?.body ? { 'Content-Type': 'application/json' } : undefined,
  })
  const body: unknown = await response.json().catch(() => ({}))
  if (!response.ok) {
    const parsed = errorSchema.safeParse(body)
    throw new Error(parsed.success ? parsed.data.error : `Request failed (${response.status})`)
  }
  return schema.parse(body)
}

const post = <T>(path: string, schema: z.ZodType<T>, body?: object) =>
  request(path, schema, { method: 'POST', body: JSON.stringify(body ?? {}) })

export const deviceApi = {
  getDevice: () => request('/api/device', deviceSchema),
  listPrinters: (connection: BambuddyConnection) => post('/api/printers', z.array(printerSchema), connection),
  saveSettings: (selection: PrinterSelection) => post('/api/settings', okSchema, selection),
  identify: () => post('/api/identify', okSchema),
  clearPlate: () => post('/api/clear-plate', okSchema),
  scanNetworks: () => request('/api/wifi/networks', z.array(networkSchema)),
  saveWifi: (credentials: WifiCredentials) => post('/api/wifi', okSchema, credentials),
  listPeers: () => request('/api/peers', z.array(peerSchema)),
}
