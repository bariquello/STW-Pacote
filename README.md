# STW-Pacote (pfSense) — System Way

Pacote interno para **pfSense CE 2.8 / Plus 24.x** que:

1. **Aplica o tema `pfSense-Systemway`** (CSS, JS, `head.inc`, logo e favicons) automaticamente.
2. **Adiciona o menu `Services → STW Backup FTP`**, que gera o `/etc/backup.sh` a partir
   de um formulário e (opcionalmente) agenda o backup diário via cron — substituindo o
   procedimento manual da Base de Conhecimento 338.

Versão atual: **0.4**

---

## 📂 Estrutura do repositório (limpa)

```
STW-Pacote/                          <- raiz do repo
├── stw_setup.sh                     <- instalador one-liner (bootstrap)
├── stw_update.sh                    <- atualizador rapido
├── build_dist.sh                    <- regera o tarball
├── stw-pacote-files.tar.gz          <- pacote (arvore relativa a /)
├── README.md
├── .gitignore
└── files/usr/local/
    ├── pkg/
    │   ├── stw_pacote.xml           <- GUI + menu + hooks
    │   └── stw_pacote.inc           <- logica (tema + backup FTP)
    ├── share/pfSense-pkg-STW-Pacote/
    │   ├── info.xml
    │   └── theme/
    │       ├── css/  (pfSense-Systemway.css, pfSense-dark.css, login.css)
    │       ├── js/   (pfSense.js, pfSenseHelpers.js, polyfills.js, traffic-graphs.js)
    │       └── www/  (head.inc + os 5 BINARIOS abaixo)
    └── etc/rc.d/stw_backup.sh
```

### ⚠️ Antes de commitar: adicione os 5 binários do tema
Coloque em `files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/`:

```
logo.png
logo-branco.png
favicon.ico
favicon-16x16.png
favicon-32x32.png
```

Depois **regere o tarball** para incluí-los:

```sh
sh build_dist.sh
```

---

## 🚀 Instalação em um pfSense (one-liner)

No firewall, **entre no shell `sh` primeiro** (evita erros do tcsh), depois:

```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
```

O script baixa tudo, aplica o tema + logo, registra o menu em **Services** e **gera o `/etc/backup.sh`**.
Ao final, recarregue a interface com **Ctrl+F5** (ou faça logout/login).

---

## 🔄 Atualizar um firewall já instalado

```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_update.sh" | sh
```

---

## 🧩 Como o backup.sh é gerado

- O `/etc/backup.sh` é **SEMPRE** gerado ao salvar o formulário em *Services → STW Backup FTP*.
- O checkbox **"Agendar backup diário (cron)"** controla apenas o agendamento — não a geração.
- Campos: HOST, Porta, Usuário, Senha, Prefixo do arquivo (`ARQUIVO`) e horário do cron.

---

## 🗑️ Desinstalar

Pelo Package Manager (se registrado) ou via shell:

```sh
sh
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");stw_pacote_deinstall();echo "removido\n";'
```

Isso restaura os arquivos core originais, remove o tema/menu e apaga o `/etc/backup.sh`.

---

*System Way — uso interno.*
