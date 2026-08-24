# pfSense-pkg-STW-Pacote (STW-Pacote)

Pacote interno **System Way** para pfSense CE 2.7 / pfSense Plus 24.x.

Ao instalar, o pacote:

1. **Aplica automaticamente o tema `pfSense-Systemway`** — faz o *deploy* dos arquivos reais de personalização em `/usr/local/www` (CSS, JS, `head.inc` e logos) e define `webguicss = pfSense-Systemway.css`.
2. **Adiciona o menu `Services → STW Backup FTP`**, que substitui o procedimento manual (Base de Conhecimento 338): gera o `/etc/backup.sh` a partir dos campos da GUI e agenda a execução **diária via cron**.

> O tema embutido é exatamente o seu: paleta verde System Way (`#0C6C5C` / `#159C4D`), tela de login customizada, branding **"Systemway Firewall"** no menu e favicons próprios.

---

## 1. Conteúdo do pacote

```
sysutils/pfSense-pkg-STW-Pacote/
├── Makefile                         # Port FreeBSD
├── pkg-descr                        # Descrição do port
├── README.md
└── files/usr/local/
    ├── pkg/
    │   ├── stw_pacote.xml           # GUI + menu em Services + hooks
    │   └── stw_pacote.inc           # Lógica: deploy do tema + backup FTP
    ├── share/pfSense-pkg-STW-Pacote/
    │   ├── info.xml                 # Manifesto
    │   └── theme/                   # << ASSETS REAIS DO TEMA (staging)
    │       ├── css/
    │       │   ├── pfSense-Systemway.css   ✅ incluído
    │       │   ├── pfSense-dark.css         ✅ incluído
    │       │   └── login.css                ✅ incluído
    │       ├── js/
    │       │   ├── pfSense.js               ✅ incluído
    │       │   ├── pfSenseHelpers.js        ✅ incluído
    │       │   ├── polyfills.js             ✅ incluído
    │       │   └── traffic-graphs.js        ✅ incluído
    │       └── www/
    │           ├── head.inc                 ✅ incluído
    │           └── COLOQUE_OS_BINARIOS_AQUI.txt
    └── etc/rc.d/stw_backup.sh        # rc script do serviço
```

### ⚠️ Falta apenas incluir os 5 binários (não vieram no upload)
Coloque em `files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/` os arquivos do seu SharePoint, **com estes nomes exatos**:

```
logo.png
logo-branco.png
favicon.ico
favicon-16x16.png
favicon-32x32.png
```

O pacote copia todos eles para `/usr/local/www/` na instalação. Se algum estiver ausente, o pacote instala normalmente (registra um aviso em log) — mas o logo/favicon do tema não aparecerá até você incluí-los e reinstalar.

---

## 2. Como o deploy do tema funciona

Na instalação (`stw_pacote_install` → `stw_deploy_theme`):

| Origem (staging)                    | Destino (live)                       | Backup? |
|-------------------------------------|--------------------------------------|:-------:|
| `theme/css/pfSense-Systemway.css`   | `/usr/local/www/css/…`               | — |
| `theme/css/pfSense-dark.css`        | `/usr/local/www/css/…`               | — |
| `theme/css/login.css`               | `/usr/local/www/css/login.css`       | — |
| `theme/js/*.js`                     | `/usr/local/www/js/…`                | ✅ |
| `theme/www/head.inc`                | `/usr/local/www/head.inc`            | ✅ |
| `theme/www/logo*.png`, `favicon*`   | `/usr/local/www/…`                   | — |

- Os arquivos **core** (`head.inc`, `pfSense.js`, `pfSenseHelpers.js`, `polyfills.js`, `traffic-graphs.js`) têm o **original respaldado** em `…/orig_backup/` antes de serem sobrescritos.
- No **uninstall** (`stw_pacote_deinstall`), os core são **restaurados** a partir do backup, os CSS/logos adicionados são removidos e o `webguicss` volta ao tema anterior.
- Após um **update do pfSense** (que sobrescreve arquivos core), basta abrir **Services → STW Backup FTP**, marcar **"Reaplicar tema pfSense-Systemway"** e salvar.

---

## 3. Compilar o pacote (FreeBSD Ports / poudriere)

Em uma máquina **FreeBSD da mesma versão do pfSense**:

```sh
cp -r sysutils/pfSense-pkg-STW-Pacote /usr/ports/sysutils/
cd /usr/ports/sysutils/pfSense-pkg-STW-Pacote
make package
# saída: work/pkg/pfSense-pkg-STW-Pacote-0.3.pkg
```

Instale no firewall:

```sh
pkg add /tmp/pfSense-pkg-STW-Pacote-0.3.pkg
```

Para distribuir a vários firewalls como um repo privado (igual aos pacotes oficiais), publique o `.pkg` num repositório `pkg` e referencie em `/usr/local/etc/pkg/repos/` → aparece em **System > Package Manager**.

---

## 4. Instalação rápida SEM compilar (teste em 1 firewall)

```sh
# via SSH no pfSense (opção 8 - Shell)
# 1. lógica + GUI
scp files/usr/local/pkg/*                 root@FW:/usr/local/pkg/
# 2. staging do tema (com os binários já incluídos!)
scp -r files/usr/local/share/pfSense-pkg-STW-Pacote  root@FW:/usr/local/share/
# 3. rc script
scp files/usr/local/etc/rc.d/stw_backup.sh root@FW:/usr/local/etc/rc.d/
chmod +x /usr/local/etc/rc.d/stw_backup.sh
```

No pfSense, entre em **Services > STW Backup FTP**, marque **"Reaplicar tema"** + preencha o backup e **Save**. Isso dispara `stw_deploy_theme()` + `stw_pacote_resync()`.

---

## 5. Validação pós-instalação

```sh
# Tema
ls -l /usr/local/www/css/pfSense-Systemway.css /usr/local/www/head.inc
grep webguicss /cf/conf/config.xml            # deve mostrar pfSense-Systemway.css

# Backup FTP
cat /etc/backup.sh            # confere valores gerados
sh /etc/backup.sh             # execução manual de teste
crontab -l | grep backup.sh   # confere o agendamento
```

Confirme no servidor FTP a chegada de `NOME-DD_MM_AAAA.xml`, `Relatorio_Diario_Backup.txt` e `Relatorio_Mensal_Backup.txt`.

---

*System Way — uso interno. Versão do pacote: 0.3*
