<script lang="ts">
  import RefreshCwIcon from '@lucide/svelte/icons/refresh-cw'
  import type { Device } from '$lib/api/device-api'
  import { Button } from '$lib/components/ui/button'
  import * as Card from '$lib/components/ui/card'
  import { Input } from '$lib/components/ui/input'
  import { Label } from '$lib/components/ui/label'
  import * as Select from '$lib/components/ui/select'
  import { Action, type DeviceManager } from '$lib/device/device-manager.svelte'

  let { manager, device }: { manager: DeviceManager; device: Device } = $props()

  const apiKeyPlaceholder = $derived(device.hasApiKey ? 'Saved — leave blank to keep' : 'bb_…')
</script>

<Card.Root>
  <Card.Header>
    <Card.Title>Bambuddy</Card.Title>
    <Card.Description>Server and printer this button controls.</Card.Description>
  </Card.Header>
  <Card.Content class="flex flex-col gap-4">
    <div class="flex flex-col gap-2">
      <Label for="bambuddy-url">Server URL</Label>
      <Input id="bambuddy-url" placeholder="http://bambuddy.local:8000" bind:value={manager.bambuddyUrl} />
    </div>
    <div class="flex flex-col gap-2">
      <Label for="api-key">API key</Label>
      <Input id="api-key" type="password" autocomplete="off" placeholder={apiKeyPlaceholder} bind:value={manager.apiKey} />
      <p class="text-muted-foreground text-xs">Needs the "Control printer" permission.</p>
    </div>
    <div class="flex flex-col gap-2">
      <Label>Printer</Label>
      <div class="flex gap-2">
        <Select.Root type="single" bind:value={manager.selectedPrinterId}>
          <Select.Trigger class="flex-1">
            {manager.selectedPrinter?.name ?? 'Select a printer'}
          </Select.Trigger>
          <Select.Content>
            {#each manager.printers as printer (printer.id)}
              <Select.Item value={String(printer.id)} label={printer.name}>
                {printer.name}
                {#if printer.model}<span class="text-muted-foreground text-xs">{printer.model}</span>{/if}
              </Select.Item>
            {/each}
          </Select.Content>
        </Select.Root>
        <Button
          variant="outline"
          size="icon"
          aria-label="Load printers"
          onclick={() => manager.loadPrinters()}
          disabled={!manager.bambuddyUrl || manager.isPending(Action.LOAD_PRINTERS)}
        >
          <RefreshCwIcon class={manager.isPending(Action.LOAD_PRINTERS) ? 'animate-spin' : ''} />
        </Button>
      </div>
    </div>
  </Card.Content>
  <Card.Footer>
    <Button
      class="w-full"
      onclick={() => manager.saveSettings()}
      disabled={!manager.canSaveSettings || manager.isPending(Action.SAVE_SETTINGS)}
    >
      Save
    </Button>
  </Card.Footer>
</Card.Root>
