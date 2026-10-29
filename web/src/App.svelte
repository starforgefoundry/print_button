<script lang="ts">
  import { onMount } from 'svelte'
  import CircleAlertIcon from '@lucide/svelte/icons/circle-alert'
  import CircleCheckIcon from '@lucide/svelte/icons/circle-check'
  import XIcon from '@lucide/svelte/icons/x'
  import * as Alert from '$lib/components/ui/alert'
  import { Button } from '$lib/components/ui/button'
  import { DeviceManager } from '$lib/device/device-manager.svelte'
  import StatusCard from './components/StatusCard.svelte'
  import BambuddyCard from './components/BambuddyCard.svelte'
  import WifiCard from './components/WifiCard.svelte'
  import PeersCard from './components/PeersCard.svelte'

  const manager = new DeviceManager()
  onMount(() => manager.start())
</script>

<main class="mx-auto flex max-w-xl flex-col gap-4 px-4 py-8">
  <header class="flex items-baseline justify-between">
    <h1 class="text-xl font-semibold tracking-tight">Print Button</h1>
    {#if manager.device}
      <span class="text-muted-foreground font-mono text-xs">{manager.device.hostname}</span>
    {/if}
  </header>

  {#if manager.unreachable}
    <Alert.Root variant="destructive">
      <CircleAlertIcon />
      <Alert.Title>Lost connection to the button</Alert.Title>
      <Alert.Description>Retrying…</Alert.Description>
    </Alert.Root>
  {/if}

  {#if manager.notice}
    <Alert.Root variant={manager.notice.kind === 'error' ? 'destructive' : 'default'} class="pr-10">
      {#if manager.notice.kind === 'error'}<CircleAlertIcon />{:else}<CircleCheckIcon />{/if}
      <Alert.Title>{manager.notice.text}</Alert.Title>
      <Button
        variant="ghost"
        size="icon-sm"
        class="absolute top-2 right-2"
        aria-label="Dismiss"
        onclick={() => manager.dismissNotice()}
      >
        <XIcon />
      </Button>
    </Alert.Root>
  {/if}

  {#if manager.device}
    <StatusCard {manager} device={manager.device} />
    <BambuddyCard {manager} device={manager.device} />
    <WifiCard {manager} device={manager.device} />
    {#if manager.device.wifiConnected}
      <PeersCard {manager} device={manager.device} />
    {/if}
    <p class="text-muted-foreground text-center font-mono text-xs">
      v{manager.device.firmwareVersion} · {manager.device.mac}
    </p>
  {:else if !manager.unreachable}
    <p class="text-muted-foreground text-sm">Loading…</p>
  {/if}
</main>
