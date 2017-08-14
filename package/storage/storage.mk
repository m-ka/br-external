################################################################################
#
# storage
#
################################################################################

STORAGE_VERSION = 1.0
STORAGE_SITE = $(BR2_EXTERNAL_BARX_PATH)/package/storage
STORAGE_SITE_METHOD = local
STORAGE_LICENSE = GPL-2.0+
STORAGE_LICENSE_FILES = COPYING

define STORAGE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/test_storage.py $(TARGET_DIR)/usr/bin/test_storage.py
endef

$(eval $(kernel-module))
$(eval $(generic-package))
