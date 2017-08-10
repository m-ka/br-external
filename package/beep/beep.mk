################################################################################
#
# beep
#
################################################################################

BEEP_VERSION = 1.3
BEEP_SITE = http://www.johnath.com/beep
BEEP_SOURCE = beep-$(BEEP_VERSION).tar.gz
BEEP_LICENSE = GPL-2.0+
BEEP_LICENSE_FILES = COPYING

define BEEP_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) beep
endef

define BEEP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/beep $(TARGET_DIR)/usr/bin/beep
endef

$(eval $(generic-package))
