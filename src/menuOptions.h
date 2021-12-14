#pragma once

/*
 * Offer the user to enter a keyword or a website and to store a password for it
 */
void storePassword(char* key);

/*
 * Allow the user to retrieve a password he previously stored
 */
void retrievePassword(char* key);

/*
 * Offer the user to delete a password
 */
void deletePassword();