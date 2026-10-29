<script lang="ts">
  import WifiIcon from '@lucide/svelte/icons/wifi'
  import LockIcon from '@lucide/svelte/icons/lock'
  import RefreshCwIcon from '@lucide/svelte/icons/refresh-cw'
  import type { Device } from '$lib/api/device-api'
  import { Badge } from '$lib/components/ui/badge'
  import { Button } from '$lib/components/ui/button'
  import * as Card from '$lib/components/ui/card'
  import { Input } from '$lib/components/ui/input'
  import { Label } from '$lib/components/ui/label'
  import { Action, type DeviceManager } from '$lib/device/device-manager.svelte'

  let { manager, device }: { manager: DeviceManager; device: Device } = $props()

  let editing = $state(false)
  const showForm = $derived(device.setupPortal || editing)
</script>

<Card.Root>
  <Card.Header>
    <Card.Title>Wi-Fi</Card.Title>
    <Card.Description class="flex items-center gap-2">
      <WifiIcon class="size-4" />
      {#if device.wifiConnected}
        {device.wifiSsid} · {device.ip} · {device.rssi} dBm
      {:else}
        Not connected{device.wifiSsid ? ` to ${device.wifiSsid}` : ''}
      {/if}
    </Card.Description>
    {#if !showForm}
      <Card.Action>
        <Button variant="outline" size="sm" onclick={() => (editing = true)}>Change</Button>
      </Card.Action>
    {/if}
  </Card.Header>
  {#if showForm}
    <Card.Content class="flex flex-col gap-4">
      <div class="flex flex-col gap-2">
        <div class="flex items-center justify-between">
          <Label>Nearby networks</Label>
          <Button
            variant="ghost"
            size="sm"
            onclick={() => manager.scanNetworks()}
            disabled={manager.isPending(Action.SCAN_NETWORKS)}
          >
            <RefreshCwIcon class={manager.isPending(Action.SCAN_NETWORKS) ? 'animate-spin' : ''} />
            Scan
          </Button>
        </div>
        {#if manager.networks.length > 0}
          <div class="flex max-h-48 flex-col overflow-y-auto rounded-md border">
            {#each manager.networks as network (network.ssid)}
              <button
                type="button"
                class="hover:bg-accent flex items-center justify-between px-3 py-2 text-left text-sm"
                onclick={() => (manager.wifiSsid = network.ssid)}
              >
                <span class="flex items-center gap-2">
                  {network.ssid}
                  {#if network.secure}<LockIcon class="text-muted-foreground size-3" />{/if}
                </span>
                {#if network.ssid === manager.wifiSsid}
                  <Badge>Selected</Badge>
                {:else}
                  <span class="text-muted-foreground font-mono text-xs">{network.rssi} dBm</span>
                {/if}
              </button>
            {/each}
          </div>
        {/if}
      </div>
      <div class="flex flex-col gap-2">
        <Label for="wifi-ssid">Network name</Label>
        <Input id="wifi-ssid" bind:value={manager.wifiSsid} />
      </div>
      <div class="flex flex-col gap-2">
        <Label for="wifi-password">Password</Label>
        <Input id="wifi-password" type="password" bind:value={manager.wifiPassword} />
      </div>
    </Card.Content>
    <Card.Footer class="gap-2">
      {#if !device.setupPortal}
        <Button variant="outline" onclick={() => (editing = false)}>Cancel</Button>
      {/if}
      <Button
        class="flex-1"
        onclick={() => manager.saveWifi()}
        disabled={!manager.wifiSsid || manager.isPending(Action.SAVE_WIFI)}
      >
        Save and reboot
      </Button>
    </Card.Footer>
  {/if}
</Card.Root>
