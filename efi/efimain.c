/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007 Intel Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301, USA.
 */

#include <grub/efi/efi.h>
#include <grub/efi/api.h>
#include <grub/efi/misc.h>
#include <grub/misc.h>

#include <shared.h>

#define GRUB_SCRATCH_MEM_PAGES  (GRUB_SCRATCH_MEM_SIZE >> 12)

/* Emulation requirements. */
void *grub_scratch_mem = NULL;

#define LOW_STACK_SIZE  0x100000
#define LOW_STACK_PAGES (LOW_STACK_SIZE >> 12)
static void *low_stack, *real_stack;

static void
real_main (void)
{
  grub_efi_loaded_image_t *loaded_image;
  char *path_name;

  loaded_image = grub_efi_get_loaded_image (grub_efi_image_handle);
  grub_get_drive_partition_from_bdev_handle (loaded_image->device_handle,
					     &boot_drive,
					     &install_partition);
  path_name = grub_efi_file_path_to_path_name (loaded_image->file_path);
  if (path_name)
    {
      grub_set_config_file (path_name);
      grub_free (path_name);
    }
  grub_load_saved_default (loaded_image->device_handle);

  init_bios_info ();
}

grub_efi_status_t
efi_main (grub_efi_handle_t image_handle, grub_efi_system_table_t *sys_tab)
{
  grub_efi_image_handle = image_handle;
  grub_efi_system_table = sys_tab;
  grub_efi_init ();

  grub_scratch_mem = grub_efi_allocate_pages (0, GRUB_SCRATCH_MEM_PAGES);
  if (! grub_scratch_mem)
    {
      grub_printf ("Failed to allocate scratch mem!\n");
      return GRUB_EFI_OUT_OF_RESOURCES;
    }

  /* If current stack reside in memory region > 2G, switch stack to a
     memory region < 2G */
  if ((unsigned long)&image_handle >= 0x80000000UL)
    {
      low_stack = grub_efi_allocate_pages (0, LOW_STACK_PAGES);
      if (! low_stack)
	{
	  grub_printf("Failed to allocate low memory stack!\n");
	  return GRUB_EFI_OUT_OF_RESOURCES;
	}

      asm volatile ("movq %%rsp, %0\n\tmovq %1, %%rsp\n"
		    : "=&r" (real_stack) : "r" (low_stack+LOW_STACK_SIZE));
    }

  real_main ();

  if (real_stack) {
    asm volatile ("movq %0, %%rsp\n" : : "r" (real_stack));

    grub_efi_free_pages ((grub_efi_physical_address_t) low_stack,
			 LOW_STACK_PAGES);
  }

  grub_efi_free_pages ((grub_efi_physical_address_t) grub_scratch_mem,
		       GRUB_SCRATCH_MEM_PAGES);
  grub_efi_fini ();

  return GRUB_EFI_SUCCESS;
}