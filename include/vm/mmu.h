#ifndef MMU
#define MMU

#include <stdbool.h>
#include <stdint.h>

#include <vm/vm_constants.h>
#include <vm/vm_prot.h>

/* Map a range of physical memory onto the specified region of
 * virtual memory. This will not allow the caller to overwrite an
 * existing mapping. */
bool map_range(uint64_t, uint64_t, uint64_t, vm_prot_t, bool);

/* Same as map_range, but allows overwriting of existing mappings */
bool map_range_force(uint64_t, uint64_t, uint64_t, vm_prot_t, bool);

/* Same as map_range, but allows for 2MB L2 block entries */
bool map_range_block(uint64_t, uint64_t, uint64_t, vm_prot_t, bool);

/* Same as map_range_block, but allows overwriting of existing
 * mappings */
bool map_range_block_force(uint64_t, uint64_t, uint64_t, vm_prot_t, bool);

#endif
