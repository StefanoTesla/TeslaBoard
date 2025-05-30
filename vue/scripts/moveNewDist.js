import fs from 'fs';
import path from 'path';

const __dirname = path.dirname(decodeURIComponent(new URL(import.meta.url).pathname));

function fixPath(p) {
  if (process.platform === "win32" && p.startsWith("/")) {
    return p.slice(1);
  }
  return p;
}

const wwwPath = path.resolve(fixPath(__dirname), '../../data/www');
const distPath = path.resolve(fixPath(__dirname), '../dist');

// Controllo esistenza cartelle
if (!fs.existsSync(wwwPath)) {
  console.error(`Error checking data folder in platformio: ${wwwPath}`);
  process.exit(1);  // Esci con errore
}

if (!fs.existsSync(distPath)) {
  console.error(`Error checking vue dist folder: ${distPath}`);
  process.exit(1);
}

function deleteFolderContent(folderPath) {
  const files = fs.readdirSync(folderPath);
  for (const file of files) {
    const curPath = path.join(folderPath, file);
    if (fs.lstatSync(curPath).isDirectory()) {
      fs.rmSync(curPath, { recursive: true, force: true });
    } else {
      fs.unlinkSync(curPath);
    }
  }
}

function copyFolderContent(src, dest) {
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }
  const files = fs.readdirSync(src);
  for (const file of files) {
    const srcFile = path.join(src, file);
    const destFile = path.join(dest, file);
    if (fs.lstatSync(srcFile).isDirectory()) {
      copyFolderContent(srcFile, destFile);
    } else {
      fs.copyFileSync(srcFile, destFile);
    }
  }
}

deleteFolderContent(wwwPath);
copyFolderContent(distPath, wwwPath);
console.log('\x1b[32m%s\x1b[0m','✅ File on platformio side are refreshed!');