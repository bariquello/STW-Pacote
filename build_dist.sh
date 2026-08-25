#!/bin/sh
# Regera o tarball. Rode na raiz do repo: sh build_dist.sh
set -e
[ -d files ] || { echo "ERRO: rode na raiz do repo (onde esta files/)"; exit 1; }
W="files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
for b in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
  [ -f "$W/$b" ] || echo "AVISO: binario ausente -> $W/$b"
done
tar -czf stw-pacote-files.tar.gz -C files usr
echo "OK: stw-pacote-files.tar.gz gerado."
