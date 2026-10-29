import { svelte } from '@sveltejs/vite-plugin-svelte'
import tailwindcss from '@tailwindcss/vite'
import path from 'node:path'
import { defineConfig } from 'vite'
import { viteSingleFile } from 'vite-plugin-singlefile'
import { firmwareHeader } from './vite-plugin-firmware-header.ts'

const DEVICE_URL = process.env.DEVICE_URL ?? 'http://printbutton.local'
const FIRMWARE_HEADER = path.resolve('../firmware/include/web-ui.h')

export default defineConfig({
  plugins: [tailwindcss(), svelte(), viteSingleFile(), firmwareHeader(FIRMWARE_HEADER)],
  resolve: {
    alias: { $lib: path.resolve('./src/lib') },
  },
  server: {
    proxy: { '/api': DEVICE_URL },
  },
})
