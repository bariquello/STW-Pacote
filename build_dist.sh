#!/bin/sh
#############################################################################
# build_dist.sh - Gera o tarball de distribuicao do STW-Pacote
# Rode na sua maquina (Linux/Mac/WSL) dentro da pasta do projeto.
# Saida: stw-pacote-files.tar.gz  (arvore relativa a / )
#############################################################################
set -e

SRC="sysutils/pfSense-pkg-STW-Pacote/files"
OUT="stw-pacote-files.tar.gz"

if [ ! -d "${SRC}" ]; then
	echo "ERRO: pasta ${SRC} nao encontrada. Rode a partir da raiz do projeto."
	exit 1
fi

# Aviso se faltarem os binarios do tema
THEME_WWW="${SRC}/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
for bin in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
	if [ ! -f "${THEME_WWW}/${bin}" ]; then
		echo "AVISO: binario ausente -> ${THEME_WWW}/${bin} (o tema instala, mas sem esse asset)"
	fi
done

# Empacota o CONTEUDO de files/ como arvore relativa a / (usr/local/...)
tar -czf "${OUT}" -C "${SRC}" usr etc 2>/dev/null || tar -czf "${OUT}" -C "${SRC}" usr

echo "OK: ${OUT} gerado."
echo "Suba este arquivo para o seu host e aponte a PKG_URL do stw_setup.sh para ele."
tar -tzf "${OUT}" | head -30
