#!/bin/sh
#############################################################################
# build_dist.sh - Regera o stw-pacote-files.tar.gz a partir de files/
# Rode na raiz do repo (onde esta a pasta files/):  sh build_dist.sh
#############################################################################
set -e
SRC="files"
OUT="stw-pacote-files.tar.gz"

[ -d "$SRC" ] || { echo "ERRO: pasta files/ nao encontrada. Rode na raiz do repo."; exit 1; }

# Aviso se faltarem binarios do tema
W="$SRC/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
for b in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
	[ -f "$W/$b" ] || echo "AVISO: binario ausente -> $W/$b"
done

tar -czf "$OUT" -C "$SRC" usr
echo "OK: $OUT gerado."
tar -tzf "$OUT" | head -20
