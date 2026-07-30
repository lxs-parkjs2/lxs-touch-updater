#include "CLXSFwupdate.h"
#include <string.h>

void print_usage() 
{
    printf("LXS Touch Firmware Updater for ChromeOS\n");
    printf("\nUsage:\n");
    printf("  lxs_touch_updater --get_current_version\n");
    printf("      Get current firmware version (must complete in <40ms)\n\n");
    printf("  lxs_touch_updater --update <fw_path>\n");
    printf("      Update firmware with specified image file\n\n");
    printf("  lxs_touch_updater --get_product_id\n");
    printf("      Get current product ID\n\n");
    printf("  lxs_touch_updater --help\n");
    printf("      Display this help message\n");
}

int main(int argc, char* argv[])
{
    int nRet = 0;
    CLXSFwupdate fwUpdate;

    // Check if arguments provided
    if (argc < 2) 
    {
        fprintf(stderr, "Error: No command specified\n\n");
        print_usage();
        return 1;
    }

    // Parse command line arguments
    if (strcmp(argv[1], "--get_current_version") == 0) 
    {
        // Fast version check for ChromeOS boot time requirement (<40ms)
        char version[64] = {0};
        if (fwUpdate.GetFirmwareVersionFast(version, sizeof(version))) 
        {
            printf("%s\n", version);
            return 0;
        }
        fprintf(stderr, "Error: Failed to get firmware version\n");
        return 1;
    }
    else if (strcmp(argv[1], "--update") == 0) 
    {
        if (argc < 3) 
        {
            fprintf(stderr, "Error: Firmware path required for --update\n");
            fprintf(stderr, "Usage: %s --update <fw_path>\n", argv[0]);
            return 1;
        }

        // Open device and perform update
        if (fwUpdate.OpenDevice())
        {
            nRet = fwUpdate.StartDownload(argv[2]);
            fwUpdate.CloseDevice();
            
            if (nRet)
            {
                fprintf(stderr, "Firmware update completed successfully\n");
                return 0;
            }
        }
        
        fprintf(stderr, "Error: Firmware update failed\n");
        return 1;
    }
    else if (strcmp(argv[1], "--get_product_id") == 0) 
    {
        // Get product ID (stable across power loss)
        uint16_t pid = 0;
        if (fwUpdate.GetProductID(&pid)) 
        {
            printf("0x%04X\n", pid);
            return 0;
        }
        fprintf(stderr, "Error: Failed to get product ID\n");
        return 1;
    }
    else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) 
    {
        print_usage();
        return 0;
    }
    else 
    {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", argv[1]);
        print_usage();
        return 1;
    }

    return 0;
}