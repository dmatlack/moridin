/**
 * @file pages.c
 *
 */
#include <mm/pages.h>
#include <mm/memory.h>

#include <mm/kmalloc.h>
#include <kernel/kprintf.h> //remove me
#include <kernel/debug.h>

#include <errno.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

struct page *phys_pages;    /* All pages in physical memory */
struct page_zone *zones;  /* Physical memory divided up into zones */

/**
 * @brief Initialize all the page_zones.
 */
void page_zones_init(void)
{
	struct page_zone *zone;

	zones = kmalloc(sizeof(struct page_zone) * MAX_ZONES);
	ASSERT_NOT_NULL(zones);

	memset(zones, 0, sizeof(struct page_zone) * MAX_ZONES);

	/*
	 * We don't do anything fancy like NUMA. There is just one contiguous page
	 * zone covering all of physical memory.
	 */
	zone = zones;
	zone->pages = phys_pages;
	zone->num_pages = phys_mem_pages;
	zone->num_free = phys_mem_pages;
	zone->index = 0;
}


/**
 * @breif Initialize the physical page management system.
 */
void pages_init(void)
{
	phys_pages = kmalloc(sizeof(struct page) * phys_mem_pages);
	ASSERT_NOT_NULL(phys_pages);

	memset(phys_pages, 0, sizeof(struct page) * phys_mem_pages);

	kprintf("phys_pages: %d pages (page list: %d KB total)\n",
			phys_mem_pages,
			phys_mem_pages * sizeof(struct page) / KB(1));

	page_zones_init();
}

/**
 * @brief Return true the the given zone contains the given address.
 */
bool zone_contains(struct page_zone *zone, size_t addr)
{
	size_t page_addr = PAGE_ALIGN_DOWN(addr);

	return ZONE_START_PAGE_ADDR(zone) <= page_addr &&
		ZONE_END_PAGE_ADDR(zone)   >= page_addr;
}

/**
 * @brief Return the zone that contains the given address.
 */
struct page_zone *zone_containing(size_t addr)
{
	struct page_zone *zone;

	for (zone = zones; zone < zones + MAX_ZONES; zone++) {
		if (zone && zone_contains(zone, addr)) {
			return zone;
		}
	}

	return NULL;
}

static struct page *__alloc_pages_at(size_t addr, unsigned long n, struct page_zone *zone)
{
	struct page *page;
	struct page *end;

	// zone lock

	end = page_struct(addr + (n * PAGE_SIZE));

	for (page = page_struct(addr); page < end; page++) {
		if (page->count) {
			return NULL;
		}
	}

	for (page = page_struct(addr); page < end; page++) {
		page_get(page);
	}

	zone->num_free -= n;

	// zone unlock

	return page_struct(addr);
}

/**
 * @brief Attempt to allocate the n physical pages starting at address addr.
 *
 * @param addr The physical address of the first page to reserve.
 * @param n The number of pages to allocate.
 */
struct page *alloc_pages_at(size_t addr, unsigned long n)
{
	TRACE("addr=0x%x, n=0x%x", addr, n);

	ASSERT(IS_PAGE_ALIGNED(addr));
	ASSERT_EQUALS(zone_containing(addr), zone_containing(addr + PAGE_SIZE * n));

	return __alloc_pages_at(addr, n, zone_containing(addr));
}

/**
 * @brief Find n contiguous (unused) pages in the zone.
 *
 * Assumes the zone lock is already held.
 */
static struct page *find_contig_pages(unsigned long n, struct page_zone *zone)
{
	unsigned long num_contig;
	struct page *start;
	struct page *page;

	start = zone->pages + zone->index;
	page = start;
	num_contig = 0;

	do {
		/*
		 * This page is in use.
		 */
		if (page->count) {
			num_contig = 0;
		}
		else {
			num_contig++;

			if (num_contig == n) {
				return page - (n - 1);
			}
		}

		zone->index++;
		if (zone->index == zone->num_pages) {
			zone->index = 0;
			num_contig = 0;
		}
		page = zone->pages + zone->index;
	} while (page != start);

	return NULL;
}

struct page *__alloc_pages(unsigned long n, struct page_zone *zone)
{
	struct page *pages;
	struct page *p;

	// zone lock

	pages = find_contig_pages(n, zone);
	if (!pages) {
		goto alloc_pages_out;
	}

	for (p = pages; p < pages + n; p++) {
		page_get(p);
	}

	zone->num_free -= n;

alloc_pages_out:
	// zone unlock
	return pages;
}

/**
 * @brief Allocate n continuous pages.
 *
 * @return
 *    NULL if n contiguous pages could not be found
 *    0 otherwise
 */
struct page *alloc_pages(unsigned long n)
{
	TRACE("n=%d", n);
	ASSERT_NOTEQUALS(n, 0);
	return __alloc_pages(n, zones);
}

void __free_pages(struct page *pages, unsigned long n, struct page_zone *zone)
{
	struct page *p;

	// zone lock

	for (p = pages; p < pages + n; p++) {
		page_put(p);
	}

	zone->num_free += n;

	// zone unlock
}

/**
 * @brief Release n contiguous pages.
 */
void free_pages(struct page *pages, unsigned long n)
{
	TRACE("pages=%p, n=%d", pages, n);
	__free_pages(pages, n, zone_containing(page_address(pages)));
}
