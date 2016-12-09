/*
 * Ruby/OpenSSL Project
 * Copyright (C) 2007, 2017 Ruby/OpenSSL Project Authors
 */
#include "ossl.h"

static VALUE mKDF, eKDF;

/*
 * call-seq:
 *   KDF.pbkdf2_hmac(pass, salt:, iterations:, length:, hash:) -> aString
 *
 * PKCS #5 PBKDF2 (Password-Based Key Derivation Function 2) in combination
 * with HMAC. Takes _pass_, _salt_ and _iterations_, and then derives a key
 * of _length_ bytes.
 *
 * For more information about PBKDF2, see RFC 2898 Section 5.2
 * (https://tools.ietf.org/html/rfc2898#section-5.2).
 *
 * === Parameters
 * pass       :: The passphrase.
 * salt       :: The salt. Salts prevent attacks based on dictionaries of common
 *               passwords and attacks based on rainbow tables. It is a public
 *               value that can be safely stored along with the password (e.g.
 *               if the derived value is used for password storage).
 * iterations :: The iteration count. This provides the ability to tune the
 *               algorithm. It is better to use the highest count possible for
 *               the maximum resistance to brute-force attacks.
 * length     :: The desired length of the derived key in octets.
 * hash       :: The hash algorithm used with HMAC for the PRF. May be a String
 *               representing the algorithm name, or an instance of
 *               OpenSSL::Digest.
 */
static VALUE
kdf_pbkdf2_hmac(int argc, VALUE *argv, VALUE self)
{
    VALUE pass, salt, opts, kwargs[4], str;
    static ID kwargs_ids[4];
    int iters, len;
    const EVP_MD *md;

    if (!kwargs_ids[0]) {
	kwargs_ids[0] = rb_intern_const("salt");
	kwargs_ids[1] = rb_intern_const("iterations");
	kwargs_ids[2] = rb_intern_const("length");
	kwargs_ids[3] = rb_intern_const("hash");
    }
    rb_scan_args(argc, argv, "1:", &pass, &opts);
    rb_get_kwargs(opts, kwargs_ids, 4, 0, kwargs);

    StringValue(pass);
    salt = StringValue(kwargs[0]);
    iters = NUM2INT(kwargs[1]);
    len = NUM2INT(kwargs[2]);
    md = GetDigestPtr(kwargs[3]);

    str = rb_str_new(0, len);
    if (!PKCS5_PBKDF2_HMAC(RSTRING_PTR(pass), RSTRING_LENINT(pass),
			   (unsigned char *)RSTRING_PTR(salt),
			   RSTRING_LENINT(salt), iters, md, len,
			   (unsigned char *)RSTRING_PTR(str)))
	ossl_raise(eKDF, "PKCS5_PBKDF2_HMAC");

    return str;
}

void
Init_ossl_kdf(void)
{
#if 0
    mOSSL = rb_define_module("OpenSSL");
    eOSSLError = rb_define_class_under(mOSSL, "OpenSSLError", rb_eStandardError);
#endif

    /*
     * Document-module: OpenSSL::KDF
     *
     * Provides functionality of various KDFs (key derivation function).
     *
     * KDF is typically used for securely deriving arbitrary length symmetric
     * keys to be used with an OpenSSL::Cipher from passwords. Another use case
     * is for storing passwords: Due to the ability to tweak the effort of
     * computation by increasing the iteration count, computation can be slowed
     * down artificially in order to render possible attacks infeasible.
     *
     * Currently, OpenSSL::KDF provides implementations for the following KDF:
     *
     * * PKCS #5 PBKDF2 (Password-Based Key Derivation Function 2) in
     *   combination with HMAC
     *
     * == Examples
     * === Generating a 128 bit key for a Cipher (e.g. AES)
     *   pass = "secret"
     *   salt = OpenSSL::Random.random_bytes(16)
     *   iter = 20_000
     *   key_len = 16
     *   key = OpenSSL::KDF.pbkdf2_hmac(pass, salt: salt, iterations: iter,
     *                                  length: key_len, hash: "sha1")
     *
     * === Storing Passwords
     *   pass = "secret"
     *   # store this with the generated value
     *   salt = OpenSSL::Random.random_bytes(16)
     *   iter = 20_000
     *   hash = OpenSSL::Digest::SHA256.new
     *   len = hash.digest_length
     *   # the final value to be stored
     *   value = OpenSSL::KDF.pbkdf2_hmac(pass, salt: salt, iterations: iter,
     *                                    length: len, hash: hash)
     *
     * == Important Note on Checking Passwords
     * When comparing passwords provided by the user with previously stored
     * values, a common mistake made is comparing the two values using "==".
     * Typically, "==" short-circuits on evaluation, and is therefore
     * vulnerable to timing attacks. The proper way is to use a method that
     * always takes the same amount of time when comparing two values, thus
     * not leaking any information to potential attackers. To compare two
     * values, the following could be used:
     *
     *   def eql_time_cmp(a, b)
     *     unless a.length == b.length
     *       return false
     *     end
     *     cmp = b.bytes
     *     result = 0
     *     a.bytes.each_with_index {|c,i|
     *       result |= c ^ cmp[i]
     *     }
     *     result == 0
     *   end
     *
     * Please note that the premature return in case of differing lengths
     * typically does not leak valuable information - when using PBKDF2, the
     * length of the values to be compared is of fixed size.
     */
    mKDF = rb_define_module_under(mOSSL, "KDF");
    /*
     * Generic exception class raised if an error occurs in OpenSSL::KDF module.
     */
    eKDF = rb_define_class_under(mKDF, "KDFError", eOSSLError);

    rb_define_module_function(mKDF, "pbkdf2_hmac", kdf_pbkdf2_hmac, -1);
}
