#pragma once

// Returns heap-allocated full URL for the latest tiny image, or NULL on error.
// Caller must free the returned string.
char *manifest_get_tiny_url(void);
