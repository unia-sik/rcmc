#ifndef _PNOO0_H_
#define _PNOO0_H_

#include "node.h"
#include "share.h"

#define PNOO_ENABLE_CONGESTION_CONTROL true
#define PNOO_ENABLE_FLOW_CONTROL true
#define PNOO_FLOW_CONTROL_SIZE (conf_noc_width + 1)

void pnoo_init(node_t *node);
void pnoo_destroy(node_t *node);

#endif

