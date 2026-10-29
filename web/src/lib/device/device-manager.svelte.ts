import { SvelteSet } from 'svelte/reactivity'
import { deviceApi, type Device, type Peer, type Printer, type WifiNetwork } from '$lib/api/device-api'

const POLL_INTERVAL_MS = 2000

export const Action = {
  LOAD_PRINTERS: 'loadPrinters',
  SAVE_SETTINGS: 'saveSettings',
  IDENTIFY: 'identify',
  CLEAR_PLATE: 'clearPlate',
  SCAN_NETWORKS: 'scanNetworks',
  SAVE_WIFI: 'saveWifi',
  LOAD_PEERS: 'loadPeers',
} as const
export type Action = (typeof Action)[keyof typeof Action]

export type NoticeKind = 'success' | 'error'

export interface Notice {
  kind: NoticeKind
  text: string
}

export class DeviceManager {
  device = $state<Device | null>(null)
  unreachable = $state(false)
  notice = $state<Notice | null>(null)
  pending = new SvelteSet<Action>()

  bambuddyUrl = $state('')
  apiKey = $state('')
  printers = $state<Printer[]>([])
  selectedPrinterId = $state('')

  networks = $state<WifiNetwork[]>([])
  wifiSsid = $state('')
  wifiPassword = $state('')

  peers = $state<Peer[]>([])

  selectedPrinter = $derived(this.printers.find((printer) => String(printer.id) === this.selectedPrinterId))
  canSaveSettings = $derived(this.bambuddyUrl.length > 0 && this.selectedPrinter !== undefined)

  start(): () => void {
    this.refresh().then(() => {
      if (this.device?.bambuddyUrl && this.device.hasApiKey) this.loadPrinters()
      if (this.device?.wifiConnected) this.loadPeers()
    })
    const timer = setInterval(() => this.refresh(), POLL_INTERVAL_MS)
    return () => clearInterval(timer)
  }

  isPending(action: Action) {
    return this.pending.has(action)
  }

  async refresh() {
    try {
      const device = await deviceApi.getDevice()
      if (!this.device) this.applyDeviceDefaults(device)
      this.device = device
      this.unreachable = false
    } catch {
      this.unreachable = true
    }
  }

  loadPrinters() {
    return this.run(Action.LOAD_PRINTERS, async () => {
      this.printers = await deviceApi.listPrinters({ bambuddyUrl: this.bambuddyUrl, apiKey: this.apiKey })
    })
  }

  saveSettings() {
    const printer = this.selectedPrinter
    if (!printer) return
    return this.run(
      Action.SAVE_SETTINGS,
      async () => {
        await deviceApi.saveSettings({
          bambuddyUrl: this.bambuddyUrl,
          apiKey: this.apiKey,
          printerId: printer.id,
          printerName: printer.name,
        })
        this.apiKey = ''
        await this.refresh()
      },
      `Linked to ${printer.name}`,
    )
  }

  identify() {
    return this.run(Action.IDENTIFY, () => deviceApi.identify(), 'Flashing white for 10 seconds')
  }

  clearPlate() {
    return this.run(
      Action.CLEAR_PLATE,
      async () => {
        await deviceApi.clearPlate()
        await this.refresh()
      },
      'Plate marked as cleared',
    )
  }

  scanNetworks() {
    return this.run(Action.SCAN_NETWORKS, async () => {
      const networks = await deviceApi.scanNetworks()
      this.networks = uniqueStrongest(networks)
    })
  }

  saveWifi() {
    return this.run(
      Action.SAVE_WIFI,
      () => deviceApi.saveWifi({ ssid: this.wifiSsid, password: this.wifiPassword }),
      `Rebooting to join ${this.wifiSsid}`,
    )
  }

  loadPeers() {
    return this.run(Action.LOAD_PEERS, async () => {
      const peers = await deviceApi.listPeers()
      this.peers = peers.sort((a, b) => a.printerName.localeCompare(b.printerName))
    })
  }

  dismissNotice() {
    this.notice = null
  }

  private applyDeviceDefaults(device: Device) {
    this.bambuddyUrl = device.bambuddyUrl
    this.wifiSsid = device.wifiSsid
    if (device.printerId > 0) {
      this.selectedPrinterId = String(device.printerId)
      this.printers = [{ id: device.printerId, name: device.printerName }]
    }
  }

  private async run(action: Action, task: () => Promise<unknown>, successText?: string) {
    this.pending.add(action)
    try {
      await task()
      if (successText) this.notice = { kind: 'success', text: successText }
    } catch (error) {
      this.notice = { kind: 'error', text: error instanceof Error ? error.message : String(error) }
    } finally {
      this.pending.delete(action)
    }
  }
}

function uniqueStrongest(networks: WifiNetwork[]): WifiNetwork[] {
  const bySsid = new Map<string, WifiNetwork>()
  for (const network of networks) {
    if (!network.ssid) continue
    const existing = bySsid.get(network.ssid)
    if (!existing || network.rssi > existing.rssi) bySsid.set(network.ssid, network)
  }
  return [...bySsid.values()].sort((a, b) => b.rssi - a.rssi)
}
