#include "rate_limiter.h"
#include <algorithm>

void RateLimiter::cleanup_old_requests()
{
    auto now = chrono::steady_clock::now();
    
    for (auto& [ip, entry] : ip_tracking)
    {
        // Remove old request timestamps outside any reasonable window
        entry.requests.erase(
            remove_if(entry.requests.begin(), entry.requests.end(),
                [now](const auto& timestamp) {
                    return chrono::duration_cast<chrono::seconds>(now - timestamp).count() > 3600; // Keep max 1 hour
                }),
            entry.requests.end()
        );
        
        // Remove entries with no requests and no ban
        if (entry.requests.empty() && !entry.is_banned)
        {
            ip_tracking.erase(ip);
        }
    }
}

bool RateLimiter::check_rate_limit(const string& ip, int max_requests, int time_window_seconds)
{
    lock_guard<mutex> lock(mtx);
    
    cleanup_old_requests();
    
    auto now = chrono::steady_clock::now();
    
    // Check if IP is currently banned
    if (ip_tracking.find(ip) != ip_tracking.end())
    {
        auto& entry = ip_tracking[ip];
        if (entry.is_banned)
        {
            if (now < entry.ban_until)
            {
                return true;  // Still banned
            }
            else
            {
                entry.is_banned = false;  // Ban expired
            }
        }
    }
    
    // Ensure entry exists
    if (ip_tracking.find(ip) == ip_tracking.end())
    {
        ip_tracking[ip] = RateLimitEntry(ip);
    }
    
    auto& entry = ip_tracking[ip];
    
    // Count requests in the time window
    int count = 0;
    auto time_threshold = now - chrono::seconds(time_window_seconds);
    
    for (const auto& timestamp : entry.requests)
    {
        if (timestamp >= time_threshold)
        {
            count++;
        }
    }
    
    // Check if limit exceeded
    if (count >= max_requests)
    {
        return true;  // Should be rate limited
    }
    
    // Record this request
    entry.requests.push_back(now);
    
    return false;
}

void RateLimiter::ban_ip(const string& ip, int ban_duration_seconds)
{
    lock_guard<mutex> lock(mtx);
    
    if (ip_tracking.find(ip) == ip_tracking.end())
    {
        ip_tracking[ip] = RateLimitEntry(ip);
    }
    
    auto& entry = ip_tracking[ip];
    entry.is_banned = true;
    entry.ban_until = chrono::steady_clock::now() + chrono::seconds(ban_duration_seconds);
}

bool RateLimiter::is_banned(const string& ip)
{
    lock_guard<mutex> lock(mtx);
    
    if (ip_tracking.find(ip) == ip_tracking.end())
    {
        return false;
    }
    
    auto& entry = ip_tracking[ip];
    
    if (entry.is_banned)
    {
        auto now = chrono::steady_clock::now();
        if (now < entry.ban_until)
        {
            return true;  // Still banned
        }
        else
        {
            entry.is_banned = false;  // Ban expired
            return false;
        }
    }
    
    return false;
}

int RateLimiter::get_request_count(const string& ip, int time_window_seconds)
{
    lock_guard<mutex> lock(mtx);
    
    if (ip_tracking.find(ip) == ip_tracking.end())
    {
        return 0;
    }
    
    auto& entry = ip_tracking[ip];
    auto now = chrono::steady_clock::now();
    auto time_threshold = now - chrono::seconds(time_window_seconds);
    
    int count = 0;
    for (const auto& timestamp : entry.requests)
    {
        if (timestamp >= time_threshold)
        {
            count++;
        }
    }
    
    return count;
}

