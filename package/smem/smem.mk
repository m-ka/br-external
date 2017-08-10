################################################################################
#
# smem
#
################################################################################

SMEM_VERSION = 1.4
SMEM_SITE = https://www.selenic.com/smem/download
SMEM_SOURCE = smem-$(SMEM_VERSION).tar.gz
SMEM_LICENSE = GPL-2.0+
SMEM_LICENSE_FILES = COPYING
SMEM_DEPENDENCIES = python

define SMEM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/smem $(TARGET_DIR)/usr/bin/smem
endef

$(eval $(generic-package))
