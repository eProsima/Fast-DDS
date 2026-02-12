#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>

#include <utils/SystemInfo.hpp>
#include <utils/threading.hpp>

#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>

#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include <thread>
#include <chrono>

namespace eprosima {
namespace fastdds {
namespace rtps {

#if !defined(FASTDDS_EMBED_LICENSE)

/**
 * @brief Function used to decode a base64 string.
 * @param input base64 string to decode.
 * @return the decoded vector of the given string.
 */
static std::vector<unsigned char> b64decode(
        const std::string input)
{
    std::vector<unsigned char> ret;

    // Array to the numeric value from a b64 char to binary value
    // In case the input string has a char not included in B64chars it will have Undefined Behaviour
    // 255 spaces are taken to avoid seg fault in this case
    const uint32_t B64index[255] =
    {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 0
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 16
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63, // 32
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0, // 48
        0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, // 64
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63, // 80
        0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 //112
    };

    size_t len = input.size();

    // Avoid trailing non-base64 characters
    while (len > 0)
    {
        const char c = input[len - 1];
        if ((c >= 'A' && c <= 'Z') ||
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '+' || c == '/' || c == '-' || c == '_' || c == '=')
        {
            // Found base64 char. Use current length.
            break;
        }

        // Move back one character
        --len;
    }

    // Return empty vector for empty or non-base64 strings
    if (len < 2)
    {
        return {};
    }

    const char* p = input.c_str();
    size_t pad1 = ((0 != len % 4) || (p[len - 1] == '=')) ? 1 : 0;
    size_t pad2 = (pad1 && (len % 4 > 2 || p[len - 2] != '=')) ? 1 : 0;
    const size_t last = (len - pad1) / 4 << 2;

    for (size_t i = 0; i < last; i += 4)
    {
        uint32_t n = B64index[(uint32_t) p[i]] << 18 |
                B64index[(uint32_t) p[i + 1]] << 12 |
                B64index[(uint32_t) p[i + 2]] << 6 |
                B64index[(uint32_t) p[i + 3]];
        ret.push_back((n >> 16) & 0xFF);
        ret.push_back((n >> 8) & 0xFF);
        ret.push_back(n & 0xFF);
    }
    if (pad1 > 0)
    {
        uint32_t n = B64index[(uint32_t) p[last]] << 18 | B64index[(uint32_t) p[last + 1]] << 12;
        ret.push_back((n >> 16) & 0xFF);
        if (pad2 > 0)
        {
            n |= B64index[(uint32_t) p[last + 2]] << 6;
            ret.push_back((n >> 8) & 0xFF);
        }
    }

    return ret;
}

#endif  // !FASTDDS_EMBED_LICENSE

static std::map<std::string, std::string> read_license_payload_vars(
        const std::vector<unsigned char>& json_data)
{
    std::map<std::string, std::string> ret;

    std::vector<std::string> json_vars {
        "license_id", "license_version", "holder",
        "issued_at", "not_before", "expires_at"
    };

    nlohmann::json j = nlohmann::json::parse(json_data);

    for (const auto& json_var: json_vars)
    {
        if (!j.contains(json_var))
        {
            throw std::runtime_error("JSON does not contain an object field named '" + json_var + "'");
        }

        ret[json_var] = j[json_var].is_string() ?
                j[json_var].get<std::string>() :
                j[json_var].dump();
    }

    // products->fastddspro
    if (!j.contains("products") || !j["products"].is_object())
    {
        throw std::runtime_error("JSON does not contain the object field named 'products'");
    }
    if (!j["products"].contains("fastddspro") || !j["products"]["fastddspro"].is_object())
    {
        throw std::runtime_error("JSON does not contain the object field named 'products->fastddspro'");
    }

    const nlohmann::json& fastddspro = j["products"]["fastddspro"];
    if (!fastddspro.contains("usage"))
    {
        throw std::runtime_error("JSON does not contain an object field named 'usage'");
    }
    ret["usage"] = fastddspro["usage"].is_string() ?
            fastddspro["usage"].get<std::string>() :
            fastddspro["usage"].dump();

    if (!fastddspro.contains("features") || !fastddspro["features"].is_array())
    {
        throw std::runtime_error("JSON does not contain an object field named 'features'");
    }
    std::string features_str = "";
    const auto& features_json = fastddspro["features"];
    for (size_t i = 0; i < features_json.size(); i++)
    {
        if (features_json[i].is_string())
        {
            features_str += features_json[i].get<std::string>();
        }
        else
        {
            features_str += features_json[i].dump();
        }

        if (i != features_json.size() - 1)
        {
            features_str += ";";
        }
    }
    ret["features"] = features_str;

    if (fastddspro.contains("running_time"))
    {
        ret["running_time"] = fastddspro["running_time"].is_string() ?
                fastddspro["running_time"].get<std::string>() :
                fastddspro["running_time"].dump();
    }

    return ret;
}

static void start_runtime_enforcer(
        std::chrono::minutes limit)
{
    if (limit <= std::chrono::minutes::zero())
    {
        return;
    }
    std::thread([limit]()
            {
                std::this_thread::sleep_for(std::chrono::seconds(limit));

                // Time's up — print and exit
                std::cout << "[License] Running time limit reached (" << static_cast<long long>(limit.count())
                          << " minute" << (limit.count() == 1 ? "" : "s") << "). Closing execution...\n";

                std::_Exit(0);
            }).detach();
}

// Parse "YYYY-MM-DDT00:00:00Z" as UTC into system_clock::time_point
static std::chrono::system_clock::time_point parse_utc_z(
        const std::string& s)
{

    // shape validation
    if (s.size() != 20 || s[4] != '-' || s[7] != '-' || s[10] != 'T' ||
            s[13] != ':' || s[16] != ':' || !(s[19] == 'Z' || s[19] == 'z'))
    {
        throw std::invalid_argument("Expected format YYYY-MM-DDTHH:MM:SSZ");
    }

    std::tm tm{};
    tm.tm_isdst = -1; // no DST info
    std::istringstream iss(s.substr(0, 19)); // (exclude 'Z')
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss.fail())
    {
        throw std::invalid_argument("Failed to parse datetime components");
    }

    // convert tm (interpreted as UTC) to time_t
    std::time_t tt;
#if defined(_WIN32)
    tt = _mkgmtime(&tm); // tm as UTC
#else
    tt = timegm(&tm); // GNU/POSIX extension
#endif // if defined(_WIN32)

    if (tt == static_cast<std::time_t>(-1))
    {
        throw std::runtime_error("convert 'tm' failed");
    }

    return std::chrono::system_clock::from_time_t(tt);
}

/**
 * compare local time to UTC-Z string (timezone-safe)
 * @returns int flag.
 *      - 0: license correct
 *      - 1: license not active
 *      - 2: license expired
 */
static int compare_local_now_to_utc_z(
        const std::string& utc_not_before,
        const std::string& utc_expires_at)
{
    const auto not_before = parse_utc_z(utc_not_before);
    const auto expires_at = parse_utc_z(utc_expires_at);
    const auto local_time = std::chrono::system_clock::now();

    return (not_before > local_time ? 1 : (local_time >= expires_at ? 2 : 0));
}

static EVP_PKEY* load_pubkey_from_bytes(
        const unsigned char* data,
        size_t len)
{
    const unsigned char* p = data;

    EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &p, static_cast<long>(len));
    if (!pkey)
    {
        throw std::runtime_error("d2i_PUBKEY failed (invalid DER?)");
    }

    return pkey;
}

static int check_license(
        bool& first_participant_created_)
{
#if defined(FASTDDS_EMBED_LICENSE)
    // When FASTDDS_EMBED_LICENSE is defined, there should be an include path
    // with an `eprosima_license.ipp` file with the embedded license, including:
    // - std::vector<unsigned char> json_data with the JSON payload
    // - std::vector<unsigned char> license_data with the signature
    #include "eprosima_license.ipp"
#else

    std::string fastddshome_val = "";
    std::string fastddshome_str = "FASTDDSHOME";
    std::string license_path = "";
    std::string json_path = "";

    if ((SystemInfo::get_env(fastddshome_str, fastddshome_val) != dds::RETCODE_OK) || fastddshome_val.empty())
    {
#ifdef __linux__
        const char* home = std::getenv("HOME");
        if (!home)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "HOME environment variable is not set.");
            return false;
        }

        fastddshome_val = std::string(home) + "/.config/fastdds";
#else
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "FASTDDSHOME environment variable is not set.");
        return false;
#endif // ifdef __linux__
    }

#ifdef _WIN32
    license_path = fastddshome_val + "\\licenses\\eprosima_license.lic";
#else
    license_path = fastddshome_val + "/licenses/eprosima_license.lic";
#endif // ifdef _WIN32

    std::vector<unsigned char> json_data{};
    std::vector<unsigned char> license_data{};

    std::ifstream file(license_path);
    if (!file)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Cannot open eprosima_license.lic file");
        return false;
    }

    std::string line;
    std::string encoded_json;
    std::string encoded_sig;

    try
    {
        std::getline(file, encoded_json);
        std::getline(file, encoded_sig);

        // load inputs from $FASTDDSHOME/licenses
        json_data = b64decode(encoded_json);
        license_data = b64decode(encoded_sig);
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Fatal: " + (std::string) e.what() + "\n");
        return false;
    }
#endif  // FASTDDS_EMBED_LICENSE

    try
    {
        unsigned char pubkey_der[] = {
            0x30, 0x82, 0x02, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
            0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x02, 0x0f, 0x00,
            0x30, 0x82, 0x02, 0x0a, 0x02, 0x82, 0x02, 0x01, 0x00, 0xc2, 0xc7, 0x63,
            0x52, 0xc2, 0xaf, 0x3f, 0x85, 0xda, 0x8c, 0xd9, 0xc2, 0xe6, 0x4d, 0x67,
            0x63, 0x81, 0x15, 0x23, 0xe0, 0x66, 0x98, 0xc4, 0x12, 0x05, 0x6f, 0xcb,
            0xfb, 0x8d, 0xc5, 0x70, 0x79, 0x7e, 0x4f, 0x52, 0xbc, 0x15, 0x17, 0xf1,
            0x43, 0x79, 0x4a, 0x36, 0x50, 0xcf, 0x8e, 0x1e, 0x3b, 0x74, 0x37, 0xcb,
            0x00, 0x06, 0x37, 0x89, 0x2f, 0x07, 0xa4, 0x26, 0xd5, 0xa2, 0x70, 0xb6,
            0x49, 0x81, 0xae, 0x23, 0xe9, 0x25, 0x5c, 0xae, 0x38, 0x6d, 0xde, 0x6a,
            0xe3, 0x97, 0xb7, 0x76, 0x9d, 0x3c, 0x75, 0xf5, 0xf9, 0x3e, 0x3e, 0xe7,
            0x07, 0x8c, 0x7a, 0x5a, 0xa8, 0xc9, 0x65, 0x73, 0x5f, 0x06, 0x3d, 0xbf,
            0x60, 0xe0, 0x95, 0xd8, 0x4a, 0x30, 0x37, 0xb1, 0xfe, 0xcf, 0xf8, 0x2f,
            0x79, 0xd0, 0x92, 0xfe, 0xcb, 0xc9, 0xc6, 0x5c, 0x37, 0xf4, 0xc7, 0xe1,
            0xd2, 0xad, 0x27, 0x36, 0x2f, 0x23, 0x61, 0x4c, 0x5b, 0x3f, 0xfe, 0x0e,
            0xf9, 0x5c, 0x5e, 0x77, 0x34, 0x09, 0x3e, 0xf0, 0x5a, 0xd7, 0xb5, 0xda,
            0x8d, 0x64, 0x76, 0xf6, 0x7d, 0xea, 0x13, 0x72, 0x8c, 0xa9, 0x20, 0xab,
            0x3b, 0xc2, 0x19, 0x0c, 0x88, 0x1b, 0x82, 0xd1, 0x61, 0x27, 0x79, 0x03,
            0xc5, 0x6c, 0x47, 0xba, 0xb2, 0xb2, 0xa0, 0x8e, 0x30, 0xf2, 0xb0, 0x55,
            0x82, 0x59, 0x4f, 0x04, 0x81, 0x42, 0x8e, 0xcc, 0x6d, 0x97, 0xfc, 0xa3,
            0x12, 0x39, 0xbf, 0xb3, 0x6e, 0x5b, 0xba, 0x58, 0xad, 0xb2, 0xce, 0x63,
            0x1a, 0xb9, 0x06, 0xf5, 0x16, 0x17, 0xeb, 0x7e, 0x81, 0x5d, 0xdc, 0xa1,
            0x65, 0x15, 0x11, 0xa0, 0x2f, 0x66, 0xce, 0x81, 0xc5, 0x27, 0xea, 0x80,
            0xe8, 0x3f, 0x6e, 0x1a, 0xf8, 0xd5, 0xec, 0xa2, 0xd7, 0xfc, 0x00, 0xef,
            0x25, 0xdd, 0x3c, 0xc1, 0x33, 0x8b, 0x0c, 0xf0, 0x13, 0x03, 0x89, 0xa3,
            0xe9, 0x5e, 0x45, 0x30, 0x42, 0x75, 0x7b, 0x78, 0x1a, 0x19, 0xd9, 0xf8,
            0x69, 0x43, 0x46, 0x36, 0x52, 0x89, 0x19, 0x29, 0x41, 0x80, 0x28, 0x17,
            0xec, 0x6a, 0x87, 0xcf, 0xa8, 0x61, 0x23, 0xd3, 0x13, 0x3b, 0xdd, 0x92,
            0x06, 0x25, 0x8c, 0x88, 0x60, 0x0a, 0x9d, 0x34, 0xf8, 0xfa, 0x15, 0x06,
            0x6e, 0xa6, 0x77, 0x2e, 0x80, 0x7d, 0x3e, 0x5c, 0x77, 0xf2, 0xd4, 0xc6,
            0xfb, 0x3d, 0xfe, 0x59, 0x82, 0x87, 0xfa, 0xb8, 0x14, 0x99, 0x47, 0x25,
            0x46, 0x85, 0x66, 0xdb, 0xf5, 0x53, 0xae, 0x70, 0xf3, 0xe4, 0x59, 0xf5,
            0x9a, 0x44, 0x92, 0x58, 0x53, 0xfc, 0x8c, 0x0d, 0x72, 0xeb, 0x0e, 0x84,
            0x7e, 0xed, 0xc0, 0xb7, 0x6d, 0xff, 0xa8, 0x8d, 0x78, 0x31, 0x20, 0x0d,
            0x25, 0x2e, 0xf2, 0x93, 0x1c, 0x5c, 0x42, 0x88, 0x0d, 0x15, 0x5f, 0x5b,
            0x88, 0x3f, 0x42, 0xd7, 0x61, 0xef, 0xb1, 0xe2, 0xdf, 0xe6, 0x9e, 0x39,
            0xc3, 0x96, 0xb5, 0x4d, 0xed, 0xe2, 0x52, 0x43, 0x32, 0x2e, 0x71, 0x2e,
            0xba, 0x36, 0x09, 0xe2, 0xcc, 0x3f, 0xaa, 0x7d, 0xaa, 0xf5, 0xb2, 0x31,
            0xb0, 0x98, 0x5b, 0x52, 0x14, 0xf8, 0x48, 0xea, 0x70, 0x5a, 0xec, 0xee,
            0xf1, 0x48, 0x9d, 0x48, 0x0d, 0x79, 0x67, 0x96, 0x62, 0xdd, 0x2f, 0xb1,
            0x46, 0xa1, 0x57, 0x90, 0x1d, 0x89, 0x6f, 0xdc, 0x38, 0xbb, 0xbe, 0xae,
            0x99, 0x86, 0x97, 0xa8, 0xb1, 0x2d, 0x20, 0x87, 0x13, 0x36, 0x90, 0xba,
            0xa4, 0xd1, 0xdf, 0x80, 0x48, 0x0a, 0xa0, 0xd7, 0x04, 0xb0, 0x41, 0x18,
            0xcd, 0xe9, 0x18, 0x77, 0xf4, 0x2e, 0x72, 0x50, 0xcd, 0x36, 0x6b, 0x54,
            0xe0, 0xae, 0xa7, 0x10, 0x3e, 0xe7, 0xfe, 0xf3, 0x6e, 0x90, 0xde, 0x4e,
            0x25, 0xec, 0x9a, 0x86, 0x0d, 0x47, 0xac, 0x1c, 0x1d, 0xa2, 0x3b, 0xec,
            0xb1, 0xfc, 0xb4, 0xd4, 0x2d, 0x02, 0x03, 0x01, 0x00, 0x01
        };
        unsigned int pubkey_der_len = 550;

        EVP_PKEY* pkey = load_pubkey_from_bytes(pubkey_der, pubkey_der_len);

        // create a DigestVerify context
        EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
        if (!mdctx)
        {
            throw std::runtime_error("EVP_MD_CTX_new failed.");
        }

        // -pkeyopt rsa_mgf1_md:sha256 and the message hash
        const EVP_MD* md = EVP_sha256();

        // init algorithm (RSA-PSS with SHA-256)
        if (EVP_DigestVerifyInit(mdctx, nullptr, md, nullptr, pkey) != 1)
        {
            EVP_PKEY_free(pkey);
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("EVP_DigestVerifyInit failed.");
        }

        EVP_PKEY_CTX* pkctx = EVP_MD_CTX_pkey_ctx(mdctx);
        if (!pkctx)
        {
            EVP_PKEY_free(pkey);
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("EVP_MD_CTX_pkey_ctx returned null");
        }

        // tune parameters (padding, MGF1 md and saltlen = 32)
        if (EVP_PKEY_CTX_set_rsa_padding(pkctx, RSA_PKCS1_PSS_PADDING) <= 0 ||
                EVP_PKEY_CTX_set_rsa_mgf1_md(pkctx, md) <= 0 ||
                EVP_PKEY_CTX_set_rsa_pss_saltlen(pkctx, 32) <= 0)
        {
            EVP_PKEY_free(pkey);
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("Setting RSA-PSS parameters failed");
        }

        // feed the raw input data ("-rawin")
        if (EVP_DigestVerifyUpdate(mdctx, json_data.data(), json_data.size()) != 1)
        {
            EVP_PKEY_free(pkey);
            EVP_MD_CTX_free(mdctx);
            throw std::runtime_error("EVP_DigestVerifyUpdate failed.");
        }

        // verify license
        int ok = EVP_DigestVerifyFinal(mdctx, license_data.data(), license_data.size());

        // free memory
        EVP_PKEY_free(pkey);
        EVP_MD_CTX_free(mdctx);

        if (ok == 1)
        {
            EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, "Signature valid.");
        }
        else if (ok == 0)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Signature: INVALID.");
            return false;
        }
        else
        {
            unsigned long err = ERR_get_error();
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Verification error: "  + (std::string) ERR_error_string(err, nullptr));
            return false;
        }

        const auto json_map = read_license_payload_vars(json_data);

        // check local time to verify the certificate beginning or expiration dates.
        const auto it_not_before = json_map.find("not_before");
        const auto it_expires_at = json_map.find("expires_at");
        if (it_not_before == json_map.end())
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Signature INVALID: 'not_before' not found.");
        }
        if (it_expires_at == json_map.end())
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT, "Signature INVALID: 'expires_at' not found.");
        }

        int date_flag = compare_local_now_to_utc_z(it_not_before->second, it_expires_at->second);
        if (date_flag == 1)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Signature INVALID: license is not yet active.");
            return false;
        }
        else if (date_flag == 2)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "Signature INVALID: license has expired.");
            return false;
        }

        if (!first_participant_created_)
        {
            first_participant_created_ = true;

            int running_time = -1;
            std::string running_time_str = "";

            // check running_time and create the thread to terminate the execution
            // when the timer of the proccess > running_time of the license
            const auto it_running_time = json_map.find("running_time");
            if (it_running_time != json_map.end())
            {
                running_time = std::stoi(it_running_time->second);
                start_runtime_enforcer(std::chrono::minutes(running_time));
                running_time_str = " [Currently execution maximum running time: " + std::to_string(running_time) +
                        " minutes]";
            }

            const auto it_holder = json_map.find("holder");
            std::cout << "License valid for holder: " << it_holder->second << running_time_str << "\n";
        }
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                "Fatal: " + (std::string) e.what() + "\n");

        unsigned long err;
        while ((err = ERR_get_error()) != 0)
        {
            EPROSIMA_LOG_ERROR(RTPS_PARTICIPANT,
                    "OpenSSL: " + ((std::string) ERR_error_string(err, nullptr)));
        }
        return false;
    }

    return true;
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
