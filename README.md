# STW-Pacote (pfSense) — System Way

Pacote interno para **pfSense CE 2.8 / Plus 24.x**:

1. Aplica o tema **pfSense-Systemway** (CSS, JS, `head.inc`, logo, favicons).
2. Cria o menu **Services → STW Backup FTP**, que gera o `/etc/backup.sh` e
   opcionalmente agenda o backup diário no cron (substitui a BC 338).

Versão: **0.5**

## Correção da v0.5
A GUI salvava a configuração numa chave diferente da que o código lia, então a
senha/usuário nunca chegavam ao `backup.sh`. Agora:
- O `<name>` do XML é `stwpacote`, batendo com o `<configpath>`.
- `stw_get_config()` procura em vários candidatos e faz varredura de fallback.
- Valores passam por `html_entity_decode()` e escape de aspas.

## Estrutura
```
STW-Pacote/
├── stw_setup.sh    ├── stw_update.sh    ├── build_dist.sh
├── stw-pacote-files.tar.gz             ├── README.md
└── files/usr/local/
    ├── pkg/ (stw_pacote.inc, stw_pacote.xml)
    ├── share/pfSense-pkg-STW-Pacote/ (info.xml, theme/{css,js,www})
    └── etc/rc.d/stw_backup.sh
```

## Instalar
```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
```

## Atualizar
```sh
sh
fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_update.sh" | sh
```

## Regerar o tarball (Windows PowerShell)
```powershell
tar -czf stw-pacote-files.tar.gz -C files usr
```

*System Way — uso interno.*
