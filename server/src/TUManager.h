/**-*-C++-*-
 * \file
 * \author Guillaume Papin <guillaume.papin@epitech.eu>
 *
 * \brief Translation Unit manager, this is used for "caching" a translation
 *        unit will be reparsed but not re-created if possible, etc.
 *
 * This file is distributed under the GNU General Public License. See
 * COPYING for details.
 */

#ifndef IRONY_MODE_SERVER_TUMANAGER_H_
#define IRONY_MODE_SERVER_TUMANAGER_H_

#include "support/NonCopyable.h"

#include <clang-c/Index.h>

#include <list>
#include <map>
#include <string>
#include <vector>

class TUManager : public util::NonCopyable {
public:
  /**
   * \brief A structure configure/tune the TUManager settings.
   *
   * Those settings aimed to allow some modifications in the way the
   * translation units are by default by the TUManager.
   *
   * Note that at construction time all values are zero/default
   * initialized.
   *
   * The following example is a setting that enable some tunings on
   * the translation unit parsing for the completion results. A use
   * case of this can be for a completion plugin. If the completion
   * plugin is activated, it will benefits from such settings.
   *
\code
TUManager           tuManager;
TUManager::Settings settings;

settings.parseTUOptions |= CXTranslationUnit_CacheCompletionResults;

TUManager::SettingsID settingsID = tuManager.registerSettings(settings);

// <use TU Manager with the settings here...>

tuManager.unregisterSettings(settingsID);
\endcode
   */
  struct Settings {
    unsigned parseTUOptions;

    Settings();

    /**
     * \brief Merge another \c Settings into this one.
     *
     * \param other
     */
    void merge(const Settings &other);

    bool equal(const Settings &other) const;
  };

  /**
   * \brief A unique ID referering to a registered setting.
   *
   * \sa registerSettings(), unregisterSettings()
   */
  typedef std::list<Settings>::iterator SettingsID;

public:
  TUManager();
  ~TUManager();

  /**
   * \brief Parse \p filename with flag \p flags.
   *
   * The first time call \c clang_parseTranslationUnit() and save the TU in the
   * member \c translationUnits_, The next call with the same \p filename will
   * call \c clang_reparseTranslationUnit().
   *
   * usage:
   * \code
   * std::vector<std::string> flags;
   * flags.push_back("-I../utils");
   * CXTranslationUnit tu = tuManager.parse("file.cpp", flags);
   *
   * if (! tu)
   *   std::cerr << "parsing translation unit failed\n";
   * \endcode
   *
   * \param filename    The filename to parse.
   * \param flags       The compiler flags (typically -I dir).
   *
   * \return The translation unit, if the parsing failed the translation unit
   *         will be \c NULL.
   */
  CXTranslationUnit parse(const std::string &filename,
                          const std::vector<std::string> &flags);

  /**
   * \brief Register some \c Settings for this particular TUManager instance.
   *
   * \param settings
   *
   * \return An object representing this registration, aims to be use by
   *         \c unregisterSettings() when the settings are no longer required.
   *
   * \sa unregisterSettings()
   */
  SettingsID registerSettings(const Settings &settings);

  /**
   * \brief Unregister the given settings generated by a previous call to
   *        \c registerSettings().
   *
   * \param settingsID  The settings to remove from this \c TUManager
   *                    instance.
   *
   * \pre \p settingsID was generated by a previous call to
   *      \c registerSetttings().
   *
   * \post Any use of \p settingsID will be invalid.
   *
   * \sa registerSetttings()
   */
  void unregisterSettings(SettingsID settingsID);

  /**
   * \brief Invalidate a given cached TU, the next use of a TU will require
   *        reparsing.
   *
   * This can be useful for example: when the flags used to compile a file have
   * changed.
   *
   * \param filename    The filename for which the associated
   *                    translation unit flags need to be invalidated.
   *
   * \sa invalidateAllCachedTUs()
   */
  void invalidateCachedTU(const std::string &filename);

  /**
   * \brief Invalidate all cached TU, the next use of a TU will require
   *        reparsing.
   *
   * \sa invalidateCachedTU()
   */
  void invalidateAllCachedTUs();

private:
  /**
   * \brief Recalculate the settings and invalidate cached translation units.
   */
  void onSettingsChanged();

  Settings computeEffectiveSettings() const;

private:
  typedef std::map<const std::string, CXTranslationUnit> TranslationUnitsMap;

private:
  CXIndex index_;
  TranslationUnitsMap translationUnits_; // cache variable
  Settings effectiveSettings_;
  std::list<Settings> settingsList_;
};

#endif /* !IRONY_MODE_SERVER_TUMANAGER_H_ */