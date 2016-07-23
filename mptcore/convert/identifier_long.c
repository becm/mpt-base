/*!
 * get value from string
 */

#include "convert.h"

/*!
 * \ingroup mptConvert
 * \brief id for 'long'
 * 
 * Platform specific ID for 'long' data type.
 * 
 * \return type ID for 'long' type
 */
extern char mpt_typeidentifier_long(void)
{
	switch (sizeof(long)) {
	  case sizeof(uint16_t): return 'n';
	  case sizeof(uint32_t): return 'i';
	  case sizeof(uint64_t): return 'x';
	  default: return 0;
	}
}
/*!
 * \ingroup mptConvert
 * \brief id for 'ulong'
 * 
 * Platform specific ID for 'unsigned long' data type.
 * 
 * \return type ID for 'unsigned long' type
 */
extern char mpt_typeidentifier_ulong(void)
{
	switch (sizeof(long)) {
	  case sizeof(uint16_t): return 'q';
	  case sizeof(uint32_t): return 'u';
	  case sizeof(uint64_t): return 't';
	  default: return 0;
	}
}
