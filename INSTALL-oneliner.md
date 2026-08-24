# STW-Pacote — Instalação com 1 comando (estilo ConexTI/UserAuth)

Este é o método **mais simples** para instalar o STW-Pacote em qualquer pfSense:
um único comando `fetch ... | sh`, igual ao instalador do ConexTI/UserAuth.

---

## Como funciona (visão geral)

```
[ Firewall pfSense ]                    [ Seu host: systemway.com.br / GitHub raw ]
      |                                              |
      |  fetch -q -o - "…/stw_setup.txt" | sh  --->  stw_setup.txt (bootstrap)
      |                                              |
      |  <--- baixa stw-pacote-files.tar.gz  -------  (tema + logica do pacote)
      |
      |  extrai em /  ->  chmod  ->  registra no config.xml  ->  aplica tema + backup
```

O script bootstrap faz **tudo automaticamente**: baixa, extrai nos caminhos certos,
registra o pacote (o menu **Services → STW Backup FTP** passa a aparecer) e roda o
hook de instalação (aplica o tema `pfSense-Systemway` e prepara a rotina de backup).

---

## Passo 1 — Preparar os arquivos (você faz UMA vez)

1. Suba os **5 binários** do tema em
   `sysutils/pfSense-pkg-STW-Pacote/files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/`
   com os nomes: `logo.png`, `logo-branco.png`, `favicon.ico`, `favicon-16x16.png`, `favicon-32x32.png`.

2. Gere o tarball de distribuição:
   ```sh
   sh installer/build_dist.sh
   # gera: stw-pacote-files.tar.gz
   ```

3. Edite a URL no topo do `installer/stw_setup.sh`:
   ```sh
   PKG_URL="https://SEU-HOST/stw/stw-pacote-files.tar.gz"
   ```

## Passo 2 — Hospedar (você faz UMA vez)

Suba os 2 arquivos para um host HTTPS acessível pelos firewalls:
- `stw_setup.sh`  → publique como **`stw_setup.txt`** (ex.: `https://systemway.com.br/stw/stw_setup.txt`)
- `stw-pacote-files.tar.gz` → no caminho apontado por `PKG_URL`

> Pode ser o seu servidor web (systemway.com.br), o mesmo host do UserAuth,
> ou o **GitHub** (use a URL "raw": `https://raw.githubusercontent.com/USUARIO/REPO/main/stw_setup.txt`).

## Passo 3 — Instalar em qualquer firewall (o comando único!)

No pfSense, em **Shell (opção 8)** ou **Diagnostics → Command Prompt**:

```sh
fetch -q -o - "https://SEU-HOST/stw/stw_setup.txt" | sh
```

Aguarde ~1 minuto. Ao final:
- Recarregue o navegador com **Ctrl+F5** → o tema pfSense-Systemway já aparece.
- Vá em **Services → STW Backup FTP**, preencha e salve o backup.

---

## Atualizações

Quando você mudar o tema ou a lógica, é só **regerar o tarball** (Passo 1.2) e
subir de novo. Nos firewalls, rodar o mesmo `fetch | sh` reinstala/atualiza
por cima (o script detecta que o pacote já existe e só atualiza os arquivos).

---

## Diferença para o repositório pkg (Package Manager)

| | `fetch | sh` (este método) | Repo pkg no GitHub Pages |
|---|:---:|:---:|
| Comando único | ✅ Sim | ➖ (1 comando p/ registrar o repo, depois via GUI) |
| Precisa compilar | ❌ Não | ✅ Sim |
| Aparece em Available Packages | ❌ Não | ✅ Sim |
| Update | rodar o `fetch|sh` de novo | botão de update na GUI |

Para a maioria dos casos MSP, o `fetch | sh` é o mais prático — é exatamente o
que o ConexTI/UserAuth faz.

---
*System Way — uso interno. STW-Pacote 0.3*
