import { defineConfig } from 'vite'
import path from 'path'
import vue from '@vitejs/plugin-vue'
import tailwindcss from '@tailwindcss/vite'
import viteCompression from 'vite-plugin-compression';


export default defineConfig({
  plugins: [vue(),tailwindcss(),viteCompression(
    {
      threshold: 0,
      deleteOriginFile:true
    })],
    build: {
      rollupOptions: {
        input: {
          main: path.resolve(__dirname, 'index.html'),
          setup: path.resolve(__dirname, 'setup.html'),
          wifi: path.resolve(__dirname, 'wifi.html')
        }
      }
    }
})
