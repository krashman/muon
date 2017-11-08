// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef ATOM_BROWSER_ATOM_BROWSER_CONTEXT_H_
#define ATOM_BROWSER_ATOM_BROWSER_CONTEXT_H_

#include <memory>
#include <string>
#include <vector>

#include "atom/browser/net/atom_network_delegate.h"
#include "brightray/browser/browser_context.h"

namespace atom {

class AtomBrowserContext : public brightray::BrowserContext {
 public:
  // Get or create the BrowserContext according to its |partition| and
  // |in_memory|. The |options| will be passed to constructor when there is no
  // existing BrowserContext.
  static AtomBrowserContext* From(
      const std::string& partition, bool in_memory,
      const base::DictionaryValue& options = base::DictionaryValue());

  // brightray::URLRequestContextGetter::Delegate:
  std::unique_ptr<net::URLRequestJobFactory> CreateURLRequestJobFactory(
      content::ProtocolHandlerMap* protocol_handlers) override;
  net::HttpCache::BackendFactory* CreateHttpCacheBackendFactory(
      const base::FilePath& base_path) override;
  std::unique_ptr<net::CertVerifier> CreateCertVerifier() override;
  net::SSLConfigService* CreateSSLConfigService() override;
  std::vector<std::string> GetCookieableSchemes() override;

  // brightray::BrowserContext:
  void RegisterPrefs(PrefRegistrySimple* pref_registry) override;

  virtual AtomNetworkDelegate* network_delegate() = 0;

 protected:
  AtomBrowserContext(const std::string& partition, bool in_memory,
                     const base::DictionaryValue& options);
  ~AtomBrowserContext() override;

 private:
  bool use_cache_;

  // Managed by brightray::BrowserContext.
  AtomNetworkDelegate* network_delegate_;

  DISALLOW_COPY_AND_ASSIGN(AtomBrowserContext);
};

}  // namespace atom

#endif  // ATOM_BROWSER_ATOM_BROWSER_CONTEXT_H_
