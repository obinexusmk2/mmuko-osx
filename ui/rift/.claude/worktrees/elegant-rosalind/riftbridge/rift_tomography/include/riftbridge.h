#ifndef RIFTBRIDGE_H
#define RIFTBRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rift.h"
#include "rift_tomogrpahy/trident.h"

//! Core encoding function from R_open
RIFT_API size_t rift_encode_duplex(const uint8_t* input, size_t len, uint8_t* output, int polarity);

//! Trident Resolution (C implementation)
RIFT_API TridentNode* trident_resolve_dependencies(const char* package_json, ResolutionStrategy strategy);

//! Obi Heart & Soul Pointer (as per your request)
//! "Obi" (Heart) represents the core resolve, "Uche" (Soul) is the knowledge pointer
typedef struct {
    void* obi_heart;  // Core resolve function pointer
    void* uche_soul;  // Knowledge/context pointer (Igbo: uche = mind/soul)
    int   eze_power;  // Authority level (Igbo: eze = king/leader)
} ObiSoulPointer;

RIFT_API ObiSoulPointer* create_obi_soul_pointer(int eze_power);
RIFT_API void resolve_with_obi_soul(ObiSoulPointer* pointer, const char* dependency_spec);

#ifdef __cplusplus
}
#endif

#endif // RIFTBRIDGE_H