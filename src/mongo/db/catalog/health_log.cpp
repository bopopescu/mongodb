/**
 *    Copyright (C) 2017 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/catalog/health_log.h"
#include "mongo/db/catalog/health_log_gen.h"
#include "mongo/db/concurrency/write_conflict_exception.h"
#include "mongo/db/db_raii.h"

namespace mongo {

namespace {
const ServiceContext::Decoration<HealthLog> getHealthLog =
    ServiceContext::declareDecoration<HealthLog>();

const int64_t kDefaultHealthlogSize = 100'000'000;

CollectionOptions getOptions(void) {
    CollectionOptions options;
    options.capped = true;
    options.cappedSize = kDefaultHealthlogSize;
    return options;
}
}

HealthLog::HealthLog() : _writer(nss, getOptions(), kMaxBufferSize) {}

void HealthLog::startup(void) {
    _writer.startup(std::string("healthlog writer"));
}

void HealthLog::shutdown(void) {
    _writer.shutdown();
}

HealthLog& HealthLog::get(ServiceContext* svcCtx) {
    return getHealthLog(svcCtx);
}

HealthLog& HealthLog::get(OperationContext* opCtx) {
    return getHealthLog(opCtx->getServiceContext());
}

bool HealthLog::log(const HealthLogEntry& entry) {
    BSONObjBuilder builder;
    OID oid;
    oid.init();
    builder.append("_id", oid);
    entry.serialize(&builder);
    return _writer.insertDocument(builder.obj());
}

const NamespaceString HealthLog::nss("local", "system.healthlog");
}
