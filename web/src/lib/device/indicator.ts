import type { IndicatorState } from '$lib/api/device-api'

interface IndicatorDisplay {
  label: string
  description: string
  dotClass: string
}

export const INDICATOR_DISPLAY: Record<IndicatorState, IndicatorDisplay> = {
  setup: {
    label: 'Setup',
    description: 'Connecting to Wi-Fi or waiting for setup',
    dotClass: 'bg-blue-500 animate-pulse',
  },
  unconfigured: {
    label: 'No printer',
    description: 'Pick a printer below',
    dotClass: 'bg-cyan-400 animate-pulse',
  },
  offline: {
    label: 'Offline',
    description: 'Printer or Bambuddy unreachable',
    dotClass: 'bg-fuchsia-900',
  },
  error: {
    label: 'Error',
    description: 'Bambuddy rejected the request',
    dotClass: 'bg-fuchsia-500 animate-pulse',
  },
  busy: {
    label: 'Printing',
    description: 'Printer is busy',
    dotClass: 'bg-red-500',
  },
  plateReady: {
    label: 'Clear plate',
    description: 'Press the button once the plate is cleared',
    dotClass: 'bg-yellow-400',
  },
  idle: {
    label: 'Ready',
    description: 'Printer is idle',
    dotClass: 'bg-green-500',
  },
}
