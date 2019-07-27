#ifndef CALVIN_HANDLER_H_
#define CALVIN_HANDLER_H_

#include <memory>
#include <string>
#include <vector>

#include "server.h"

namespace calvin {

void handle(const TcpConnection &conn);

} // namespace calvin

#endif // CALVIN_HANDLER_H_
