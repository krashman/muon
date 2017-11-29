// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/extensions/atom_extensions_network_delegate.h"

#include "base/stl_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/renderer_host/chrome_navigation_ui_data.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/resource_request_info.h"
#include "extensions/browser/api/web_request/web_request_api.h"
#include "extensions/browser/extension_navigation_ui_data.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/browser/process_manager.h"
#include "net/url_request/url_request.h"

namespace extensions {

namespace {
bool g_accept_all_cookies = true;
}

AtomExtensionsNetworkDelegate::AtomExtensionsNetworkDelegate(
      Profile* browser_context) {
  browser_context_ = browser_context;
}

AtomExtensionsNetworkDelegate::~AtomExtensionsNetworkDelegate() {}

void AtomExtensionsNetworkDelegate::set_extension_info_map(
    extensions::InfoMap* extension_info_map) {
  extension_info_map_ = extension_info_map;
}

void AtomExtensionsNetworkDelegate::SetAcceptAllCookies(bool accept) {
  g_accept_all_cookies = accept;
}

void AtomExtensionsNetworkDelegate::RunCallback(
                                  base::Callback<int(void)> internal_callback,
                                  const uint64_t request_id,
                                  int previous_result) {
  if (!ContainsKey(callbacks_, request_id))
    return;

  if (previous_result == net::OK) {
    int result = internal_callback.Run();

    if (result != net::ERR_IO_PENDING) {
      // nothing ran the original callback
      callbacks_[request_id].Run(net::OK);
    }
  } else {
    // nothing ran the original callback
    callbacks_[request_id].Run(previous_result);
  }

  // make sure this don't run again
  callbacks_.erase(request_id);
}

int AtomExtensionsNetworkDelegate::OnBeforeURLRequestInternal(
    net::URLRequest* request,
    GURL* new_url) {
  return atom::AtomNetworkDelegate::OnBeforeURLRequest(
      request, callbacks_[request->identifier()], new_url);
}

int AtomExtensionsNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {

  callbacks_[request->identifier()] = callback;

  base::Callback<int(void)> internal_callback = base::Bind(
          &AtomExtensionsNetworkDelegate::OnBeforeURLRequestInternal,
          base::Unretained(this),
          request,
          new_url);

  auto ui_wrapper = base::Bind(&AtomExtensionsNetworkDelegate::RunCallback,
                                base::Unretained(this),
                                internal_callback,
                                request->identifier());

  int result = ExtensionWebRequestEventRouter::GetInstance()->OnBeforeRequest(
                browser_context_,
                extension_info_map_.get(),
                request,
                ui_wrapper,
                new_url);

  if (result == net::ERR_IO_PENDING)
    return result;

  return internal_callback.Run();
}

int AtomExtensionsNetworkDelegate::OnBeforeStartTransactionInternal(
    net::URLRequest* request,
    net::HttpRequestHeaders* headers) {
  return atom::AtomNetworkDelegate::OnBeforeStartTransaction(
    request,
    callbacks_[request->identifier()],
    headers);
}

int AtomExtensionsNetworkDelegate::OnBeforeStartTransaction(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) {

  callbacks_[request->identifier()] = callback;

  base::Callback<int(void)> internal_callback = base::Bind(
          &AtomExtensionsNetworkDelegate::OnBeforeStartTransactionInternal,
          base::Unretained(this),
          request,
          headers);

  auto ui_wrapper = base::Bind(&AtomExtensionsNetworkDelegate::RunCallback,
                                base::Unretained(this),
                                internal_callback,
                                request->identifier());

  int result = ExtensionWebRequestEventRouter::GetInstance()->
      OnBeforeSendHeaders(browser_context_,
                          extension_info_map_.get(),
                          request,
                          ui_wrapper,
                          headers);

  if (result == net::ERR_IO_PENDING)
    return result;

  return internal_callback.Run();
}

void AtomExtensionsNetworkDelegate::OnStartTransaction(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) {
  atom::AtomNetworkDelegate::OnStartTransaction(request, headers);
  ExtensionWebRequestEventRouter::GetInstance()->OnSendHeaders(
      browser_context_, extension_info_map_.get(),
      request, headers);
}

int AtomExtensionsNetworkDelegate::OnHeadersReceivedInternal(
    net::URLRequest* request,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {
  return atom::AtomNetworkDelegate::OnHeadersReceived(
                                          request,
                                          callbacks_[request->identifier()],
                                          original_response_headers,
                                          override_response_headers,
                                          allowed_unsafe_redirect_url);
}

int AtomExtensionsNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url) {

  callbacks_[request->identifier()] = callback;

  base::Callback<int(void)> internal_callback =
      base::Bind(&AtomExtensionsNetworkDelegate::OnHeadersReceivedInternal,
          base::Unretained(this),
          request,
          base::RetainedRef(original_response_headers),
          override_response_headers,
          allowed_unsafe_redirect_url);

  auto ui_wrapper = base::Bind(&AtomExtensionsNetworkDelegate::RunCallback,
                                base::Unretained(this),
                                internal_callback,
                                request->identifier());

  int result = ExtensionWebRequestEventRouter::GetInstance()->OnHeadersReceived(
    browser_context_,
    extension_info_map_.get(),
    request,
    ui_wrapper,
    original_response_headers,
    override_response_headers,
    allowed_unsafe_redirect_url);

  if (result == net::ERR_IO_PENDING)
    return result;

  return internal_callback.Run();
}

void AtomExtensionsNetworkDelegate::OnBeforeRedirect(
    net::URLRequest* request,
    const GURL& new_location) {
  atom::AtomNetworkDelegate::OnBeforeRedirect(request, new_location);
  ExtensionWebRequestEventRouter::GetInstance()->OnBeforeRedirect(
      browser_context_, extension_info_map_.get(),
      request, new_location);
}

void AtomExtensionsNetworkDelegate::OnResponseStarted(net::URLRequest* request,
                                                      int net_error) {
  atom::AtomNetworkDelegate::OnResponseStarted(request, net_error);
  ExtensionWebRequestEventRouter::GetInstance()->OnResponseStarted(
      browser_context_, extension_info_map_.get(), request, net_error);
}

void AtomExtensionsNetworkDelegate::OnCompleted(
    net::URLRequest* request,
    bool started) {
  callbacks_.erase(request->identifier());
  atom::AtomNetworkDelegate::OnCompleted(request, started);

  if (request->status().status() == net::URLRequestStatus::SUCCESS) {
    bool is_redirect = request->response_headers() &&
        net::HttpResponseHeaders::IsRedirectResponseCode(
            request->response_headers()->response_code());
    if (!is_redirect) {
      ExtensionWebRequestEventRouter::GetInstance()->OnCompleted(
          browser_context_, extension_info_map_.get(),
          request);
    }
    return;
  }

  if (request->status().status() == net::URLRequestStatus::FAILED ||
      request->status().status() == net::URLRequestStatus::CANCELED) {
    ExtensionWebRequestEventRouter::GetInstance()->OnErrorOccurred(
        browser_context_, extension_info_map_.get(),
        request, started);
    return;
  }

  NOTREACHED();
}

void AtomExtensionsNetworkDelegate::OnURLRequestDestroyed(
    net::URLRequest* request) {
  callbacks_.erase(request->identifier());
  atom::AtomNetworkDelegate::OnURLRequestDestroyed(request);
  ExtensionWebRequestEventRouter::GetInstance()->OnURLRequestDestroyed(
      browser_context_, request);
}

void AtomExtensionsNetworkDelegate::OnPACScriptError(
    int line_number,
    const base::string16& error) {
}

net::NetworkDelegate::AuthRequiredResponse
AtomExtensionsNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) {
  return ExtensionWebRequestEventRouter::GetInstance()->OnAuthRequired(
      browser_context_, extension_info_map_.get(),
      request, auth_info, callback,
      credentials);
}

}  // namespace extensions
