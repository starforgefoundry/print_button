import { mkdirSync, readFileSync, writeFileSync } from 'node:fs'
import path from 'node:path'
import { gzipSync } from 'node:zlib'
import type { Plugin } from 'vite'

const BYTES_PER_LINE = 16

export function firmwareHeader(outputFile: string): Plugin {
  let outDir = ''
  return {
    name: 'firmware-header',
    apply: 'build',
    configResolved(config) {
      outDir = path.resolve(config.root, config.build.outDir)
    },
    closeBundle() {
      const html = readFileSync(path.join(outDir, 'index.html'))
      const gzipped = gzipSync(html, { level: 9 })
      const lines: string[] = []
      for (let i = 0; i < gzipped.length; i += BYTES_PER_LINE) {
        const chunk = [...gzipped.subarray(i, i + BYTES_PER_LINE)]
        lines.push('  ' + chunk.map((byte) => `0x${byte.toString(16).padStart(2, '0')}`).join(', ') + ',')
      }
      const header = [
        '#pragma once',
        '',
        '#include <Arduino.h>',
        '',
        `const size_t WEB_UI_GZ_LENGTH = ${gzipped.length};`,
        'const uint8_t WEB_UI_GZ[] PROGMEM = {',
        ...lines,
        '};',
        '',
      ].join('\n')
      mkdirSync(path.dirname(outputFile), { recursive: true })
      writeFileSync(outputFile, header)
      console.log(`firmware header: ${html.length} B -> ${gzipped.length} B gzipped`)
    },
  }
}
