#pragma once
// Lectura/escritura del formato JSON de hardpoints (docs/Datos/Formato JSON de hardpoints.md)
#include "core/Suspension.h"
#include <string>

namespace susp {

// Devuelve true si se ha leído bien. En caso de error, 'error' explica el motivo.
bool loadSuspensionJson(const std::string& path, Suspension& out, std::string& error);
bool saveSuspensionJson(const std::string& path, const Suspension& s, std::string& error);

} // namespace susp
