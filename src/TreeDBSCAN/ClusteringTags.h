#ifndef __CLUSTERING_TAGS_H__
#define __CLUSTERING_TAGS_H__

#include <MRNet_tags.h>

/* Define here all message tags used during the TreeDBSCAN protocol */
enum 
{
   TAG_CLUSTERING_CONFIG=FirstProtocolTag,
   TAG_HULL,
   TAG_ALL_HULLS_SENT,
   TAG_NOISE,
   TAG_ALL_NOISE_SENT,
   TAG_XCHANGE_DIMENSIONS
};

#endif /* __CLUSTERING_TAGS_H__ */
