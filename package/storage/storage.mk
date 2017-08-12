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

$(eval $(kernel-module))
$(eval $(generic-package))
