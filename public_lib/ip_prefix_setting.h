#ifndef __IP_PREFIX_SETTING_H_
#define __IP_PREFIX_SETTING_H_

/*
 * This is to used in mininet for assigning ip prefixes for hosts
 */

#define NET_IDENTITY_BITS 8
#define GET_NET_IDENTITY(x) (x>>(32-NET_IDENTITY_BITS))

#endif
