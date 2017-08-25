#ifndef FORTCONF_H
#define FORTCONF_H

typedef struct fort_conf {
  UINT32 filter_disabled	: 1;
  UINT32 ip_include_all		: 1;
  UINT32 ip_exclude_all		: 1;
  UINT32 app_log_blocked	: 1;
  UINT32 app_block_all		: 1;
  UINT32 app_allow_all		: 1;
  UINT32 group_bits		: 16;

  UINT16 conf_version;
  UINT16 data_off;

  UINT16 ip_include_n;
  UINT16 ip_exclude_n;

  UINT16 apps_n;
  UINT16 groups_n;

  UINT32 app_perms_block_mask;
  UINT32 app_perms_allow_mask;

  UINT32 ip_from_include_off;
  UINT32 ip_to_include_off;

  UINT32 ip_from_exclude_off;
  UINT32 ip_to_exclude_off;

  UINT32 apps_perms_off;
  UINT32 apps_off;
  UINT32 groups_off;

  UCHAR data[4];
} FORT_CONF, *PFORT_CONF;

#define FORT_CONF_VERSION		1
#define FORT_CONF_DATA_OFF		offsetof(FORT_CONF, data)
#define FORT_CONF_IP_MAX		(1 * 1024 * 1024)
#define FORT_CONF_GROUP_MAX		16
#define FORT_CONF_GROUP_NAME_MAX	256
#define FORT_CONF_GROUPS_LEN_MAX	(FORT_CONF_GROUP_MAX * FORT_CONF_GROUP_NAME_MAX)
#define FORT_CONF_APPS_LEN_MAX		(64 * 1024 * 1024)
#define FORT_CONF_APP_PATH_MAX		(2 * 1024)
#define FORT_CONF_STR_ALIGN		4

#endif FORTCONF_H