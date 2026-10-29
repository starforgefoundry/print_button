<script lang="ts">
  import LightbulbIcon from '@lucide/svelte/icons/lightbulb'
  import CheckIcon from '@lucide/svelte/icons/check'
  import type { Device } from '$lib/api/device-api'
  import { Button } from '$lib/components/ui/button'
  import * as Card from '$lib/components/ui/card'
  import { Action, type DeviceManager } from '$lib/device/device-manager.svelte'
  import { INDICATOR_DISPLAY } from '$lib/device/indicator'

  let { manager, device }: { manager: DeviceManager; device: Device } = $props()

  const display = $derived(INDICATOR_DISPLAY[device.indicator])
  const detail = $derived(device.error || display.description)
</script>

<Card.Root>
  <Card.Content class="flex items-center gap-4">
    <span class="relative flex size-12 shrink-0 items-center justify-center rounded-full bg-muted">
      <span class="size-7 rounded-full shadow-inner {display.dotClass}"></span>
    </span>
    <div class="min-w-0 flex-1">
      <p class="font-medium">{display.label}</p>
      <p class="text-muted-foreground truncate text-sm">
        {device.printerName || 'No printer'}{device.printerState ? ` · ${device.printerState}` : ''}
      </p>
      <p class="text-muted-foreground truncate text-xs">{detail}</p>
    </div>
  </Card.Content>
  <Card.Footer class="gap-2">
    <Button variant="outline" onclick={() => manager.identify()} disabled={manager.isPending(Action.IDENTIFY)}>
      <LightbulbIcon />
      Identify
    </Button>
    {#if device.indicator === 'plateReady'}
      <Button onclick={() => manager.clearPlate()} disabled={manager.isPending(Action.CLEAR_PLATE)}>
        <CheckIcon />
        Mark plate cleared
      </Button>
    {/if}
  </Card.Footer>
</Card.Root>
