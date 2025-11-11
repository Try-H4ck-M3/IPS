#pragma once
#include "../includes.h"

struct RateLimitEntry {
    string ip;
    vector<chrono::steady_clock::time_point> requests;  // Timestamps of requests
    chrono::steady_clock::time_point ban_until;        // When ban expires
    bool is_banned;
    
    // Default constructor
    RateLimitEntry() : is_banned(false) {}
    
    // Constructor with IP
    RateLimitEntry(const string& ip_addr) : ip(ip_addr), is_banned(false) {}
};

class RateLimiter {
private:
    map<string, RateLimitEntry> ip_tracking;  // IP -> entry
    mutex mtx;
    
    void cleanup_old_requests();
    
public:
    // Check if IP should be rate limited
    // Returns true if IP is banned or exceeded rate limit
    bool check_rate_limit(const string& ip, int max_requests, int time_window_seconds);
    
    // Ban an IP for specified duration in seconds
    void ban_ip(const string& ip, int ban_duration_seconds);
    
    // Check if IP is currently banned
    bool is_banned(const string& ip);
    
    // Get current request count for an IP in the time window
    int get_request_count(const string& ip, int time_window_seconds);
};

