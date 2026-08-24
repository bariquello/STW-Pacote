# Port: pfSense-pkg-STW-Pacote
# System Way - uso interno

PORTNAME=	pfSense-pkg-STW-Pacote
PORTVERSION=	0.3
CATEGORIES=	sysutils
MASTER_SITES=	# empty
DISTFILES=	# empty
EXTRACT_ONLY=	# empty

MAINTAINER=	suporte@systemway.com.br
COMMENT=	STW: aplica tema pfSense-Systemway e menu de Backup FTP (System Way)

LICENSE=	APACHE20

RUN_DEPENDS=	${LOCALBASE}/sbin/pfSsh.php:security/pfSense-repoc-static

NO_ARCH=	yes
NO_BUILD=	yes
PLIST=		${WRKDIR}/pkg-plist

SUB_FILES=	pkg-install pkg-deinstall
SUB_LIST=	PORTNAME=${PORTNAME}

STW_SHARE=	${PREFIX}/share/${PORTNAME}

do-extract:
	${MKDIR} ${WRKSRC}

do-install:
	# GUI + logica do pacote
	${MKDIR} ${STAGEDIR}${PREFIX}/pkg
	${CP} ${FILESDIR}/usr/local/pkg/stw_pacote.xml ${STAGEDIR}${PREFIX}/pkg/
	${CP} ${FILESDIR}/usr/local/pkg/stw_pacote.inc ${STAGEDIR}${PREFIX}/pkg/
	# Manifesto
	${MKDIR} ${STAGEDIR}${STW_SHARE}
	${CP} ${FILESDIR}/usr/local/share/${PORTNAME}/info.xml ${STAGEDIR}${STW_SHARE}/
	# Staging do tema (assets sao entregues aqui e o .inc copia p/ /usr/local/www no install)
	${MKDIR} ${STAGEDIR}${STW_SHARE}/theme/css
	${MKDIR} ${STAGEDIR}${STW_SHARE}/theme/js
	${MKDIR} ${STAGEDIR}${STW_SHARE}/theme/www
	${CP} -r ${FILESDIR}/usr/local/share/${PORTNAME}/theme/css/* ${STAGEDIR}${STW_SHARE}/theme/css/
	${CP} -r ${FILESDIR}/usr/local/share/${PORTNAME}/theme/js/*  ${STAGEDIR}${STW_SHARE}/theme/js/
	${CP} -r ${FILESDIR}/usr/local/share/${PORTNAME}/theme/www/* ${STAGEDIR}${STW_SHARE}/theme/www/
	# rc script do servico
	${MKDIR} ${STAGEDIR}${PREFIX}/etc/rc.d
	${CP} ${FILESDIR}/usr/local/etc/rc.d/stw_backup.sh ${STAGEDIR}${PREFIX}/etc/rc.d/

.include <bsd.port.mk>
