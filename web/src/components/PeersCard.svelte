<script lang="ts">
  import RefreshCwIcon from '@lucide/svelte/icons/refresh-cw'
  import type { Device } from '$lib/api/device-api'
  import { Badge } from '$lib/components/ui/badge'
  import { Button } from '$lib/components/ui/button'
  import * as Card from '$lib/components/ui/card'
  import { Action, type DeviceManager } from '$lib/device/device-manager.svelte'

  let { manager, device }: { manager: DeviceManager; device: Device } = $props()
</script>

<Card.Root>
  <Card.Header>
    <Card.Title>Buttons on this network</Card.Title>
    <Card.Description>Open another button to set it up.</Card.Description>
    <Card.Action>
      <Button
        variant="ghost"
        size="icon-sm"
        aria-label="Refresh"
        onclick={() => manager.loadPeers()}
        disabled={manager.isPending(Action.LOAD_PEERS)}
      >
        <RefreshCwIcon class={manager.isPending(Action.LOAD_PEERS) ? 'animate-spin' : ''} />
      </Button>
    </Card.Action>
  </Card.Header>
  <Card.Content>
    {#if manager.peers.length === 0}
      <p class="text-muted-foreground text-sm">
        {manager.isPending(Action.LOAD_PEERS) ? 'Searching…' : 'No buttons found.'}
      </p>
    {:else}
      <div class="flex flex-col divide-y rounded-md border">
        {#each manager.peers as peer (peer.hostname)}
          <a href="http://{peer.ip}/" class="hover:bg-accent flex items-center justify-between px-3 py-2 text-sm">
            <span class="flex flex-col">
              <span class="font-mono text-xs">{peer.hostname}</span>
              <span class="text-muted-foreground text-xs">{peer.ip}</span>
            </span>
            {#if peer.ip === device.ip}
              <Badge variant="secondary">This button</Badge>
            {:else if peer.printerName}
              <Badge variant="outline">{peer.printerName}</Badge>
            {:else}
              <Badge variant="destructive">Unassigned</Badge>
            {/if}
          </a>
        {/each}
      </div>
    {/if}
  </Card.Content>
</Card.Root>
