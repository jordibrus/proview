/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2013 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

#include "pwr.h"
#include "rt_qcom.h"
#include "rt_qdb.h"
#include "rt_qini.h"
#include "rt_qdb_msg.h"
#include "co_tree.h"
#include "co_cdh.h"

main () {
  pwr_tStatus	sts;
  tree_sTable	*nodes;
  qini_sNode	*nep;
  qini_sNode	*me = NULL;
  char		*s;
  FILE		*f;
  char		my_name[80];
  int		len = sizeof(my_name);
  int		errors;

  gethostname(my_name, len);
  if ((s = strchr(my_name, '.')) != NULL)
    *s = '\0';

  printf("-- Starting QCOM on node: %s\n", my_name);

  f = fopen("sys$login:ld_hosts_001.dat", "r");
  if (f == NULL) {
    perror("sys$login:ld_hosts_001.dat");
    exit(1);
  }

  nodes = tree_CreateTable(sizeof(pwr_tNodeId), offsetof(qini_sNode, nid),
    sizeof(qini_sNode), 10, tree_eComp_nid, NULL);

  errors = qini_ParseFile(f, nodes);
  if (errors != 0) {
    printf("** %d errors where found, qcom will not be started!\n", errors);
    exit(1);
  }
  
  for (
    nep = tree_Minimum(nodes);
    nep != NULL;
    nep = tree_Successor(nodes, nep)
  ) { 
    printf("%s %d %d %d %d\n", nep->name, nep->nid, nep->naddr, nep->port,
      nep->connect);
    if (strcmp(my_name, nep->name) == 0) {
      printf("-- My nid is: %d\n", nep->nid);
      me = nep;
    }
  }

  if (me == NULL) {
    printf("** Could not find myself!\n");
    exit(1);
  }

  qini_BuildDb(&sts, nodes, me, NULL, 1);

  qdb->g->log.m = 7;

  sleep(20000000);
}
