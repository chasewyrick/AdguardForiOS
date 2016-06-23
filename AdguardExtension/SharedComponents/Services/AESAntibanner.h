/**
    This file is part of Adguard for iOS (https://github.com/AdguardTeam/AdguardForiOS).
    Copyright © 2015 Performix LLC. All rights reserved.

    Adguard for iOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Adguard for iOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Adguard for iOS.  If not, see <http://www.gnu.org/licenses/>.
*/
#import <Foundation/Foundation.h>

#define AS_EXECUTION_PERIOD_TIME                           3600 // 1 hours
#define AS_EXECUTION_LEEWAY                                5 // 5 seconds
#define AS_EXECUTION_DELAY                                 2 // 2 seconds

#define AS_CHECK_FILTERS_UPDATES_PERIOD                    AS_EXECUTION_PERIOD_TIME
#define AS_CHECK_FILTERS_UPDATES_FROM_UI_DELAY             AS_EXECUTION_DELAY
#define AS_CHECK_FILTERS_UPDATES_LEEWAY                    AS_EXECUTION_LEEWAY
#define AS_CHECK_FILTERS_UPDATES_DEFAULT_PERIOD            AS_EXECUTION_PERIOD_TIME*6

/// Timeout for downloading of data from the remote services
#define AS_URL_LOAD_TIMEOUT                                60

/// When anitbanner service installed
extern NSString *ASAntibannerInstalledNotification;

/// When anitbanner service did not installed
extern NSString *ASAntibannerNotInstalledNotification;

/// When anitbanner service ready to work
extern NSString *ASAntibannerReadyNotification;

/// When antibanner filter metadata updated
extern NSString *ASAntibannerUpdateFilterMetadataNotification;

/// When antibanner filter rules updated
extern NSString *ASAntibannerUpdateFilterRulesNotification;

/// When anitbanner started update process
extern NSString *ASAntibannerStartedUpdateNotification;

/// When anitbanner finished update process
extern NSString *ASAntibannerFinishedUpdateNotification;
/// Key for userInfo of ASAntibannerFinishedUpdateNotification that defines
/// array of metadata objects of updated filters.
extern NSString *ASAntibannerUpdatedFiltersKey;

/// When anti banner update process failed because backend service is unreachable
extern NSString *ASAntibannerFailuredUpdateNotification;

/// When antibanner filter updated from Ad Block Preferences and Main Panel(enabled/desabled/unsubscribe etc..)
extern NSString *ASAntibannerUpdateFilterFromUINotification;

@class Reachability, ASDFilterRule, ACLJobController, ACLExecuteBlockDelayed;

/////////////////////////////////////////////////////////////////////////
#pragma mark - ASAntibanner
/////////////////////////////////////////////////////////////////////////

/**
 Service implements: updating from backend, auto detect filters, storing info and rules
 for Ad Blocker (the same Antibanner, requestFilter), and so on..
 */
@interface AESAntibanner : NSObject

/////////////////////////////////////////////////////////////////////////
#pragma mark Properties and public methods
/////////////////////////////////////////////////////////////////////////

/**
 Enable/Disable periodical processes, like updates or autodetect.
 */
@property BOOL enabled;

/**
 Indicates that antibanner updates of filters right now.
 */
@property (readonly) BOOL updatesRightNow;

/**
 Indicator that metadata and count of filters are out of date.
 Used for subscribing of new filters.
 */
@property (nonatomic, readonly) BOOL metadataForSubscribeOutdated;

/**
 Obtain rules for active (enabled) filters.
 Rules from USER filter will be returned last in list.
 
 @return Array of ASDFilterRule objects that represent rules of filtering.
 Returns empty array if receiver property "enabled" equal NO.
 */
- (NSArray *)activeRules;

/**
 Obtain active rules for active (enabled) filter.
 
 @param filterId    Filter Id, which will be checked
 and whose rules will be returned.
 
 @return Array of ASDFilterRule objects that represent rules of filtering.
 Returns empty array if receiver property "enabled" equal NO.
 */
- (NSArray *)activeRulesForFilter:(NSNumber *)filterId;

/**
 Obtain groups information.
 @return Array of ASDFilterGroup objects that contains
 all stored in database groups.
 */
- (NSArray *)groups;

/**
 Obtain filters information.
 @return Array of ASDFilterMetadata objects that contains
 all stored in database antibanner filters.
 */
- (NSArray *)filters;

/**
 Obtain rules for filter.
 @return Array of ASDFilterRule objects that contains
 all rules stored in database for antibanner filter,
 which is defined filterId parameter.
 */
- (NSArray *)rulesForFilter:(NSNumber *)filterId;

/**
 Set status of antibanner filter.
 
 @param filterId     Identificator of filter (filter Id).
 @param enabled      Enable/Disable of filter.
 @param  fromUI      Indicator that setting of filters status is performed from UI.
 */
- (void)setFilter:(NSNumber *)filterId enabled:(BOOL)enabled fromUI:(BOOL)fromUI;

/**
 
 Sets status of rules in antibanner filter.
 
 @param ruleIds     List of NSNumber objects. Identificators of rules.
 @param filterId    Identificator of filter (filter Id).
 @param enabled     Enable/Disable of rule.
 */
- (void)setRules:(NSArray *)ruleIds filter:(NSNumber *)filterId enabled:(BOOL)enabled;

/**
 Checks that antibanner filter is editable,
 and then adds rule for antibanner filter into DB.
 
 @return YES if success.
 */
- (BOOL)addRule:(ASDFilterRule *)rule;

/**
 Checks that antibanner filter is editable,
 and then updates rule for antibanner filter into DB,
 using filterId and ruleId as key.
 
 @return YES if success.
 */
- (BOOL)updateRule:(ASDFilterRule *)rule;

/**
 Checks that antibanner filter is editable,
 and then removes all rules of the antibanner filter from DB,
 and then adds new rules from list.
 
 @param ruleTexts    List of NSString objects.
 Texts of rules, which must be added into filter.
 @param filterId     Identificator of filter (filter Id).
 
 @return YES if success.
 */
- (BOOL)importRuleTexts:(NSArray *)ruleTexts filterId:(NSNumber *)filterId;

/**
 Checks that antibanner filter is editable,
 and then removes rules for antibanner filter from DB.
 
 @param ruleIds     List of NSNumber objects. Identificators of rules.
 @param filterId    Identificator of filter (filter Id).
 
 @return YES if success.
 */
- (BOOL)removeRules:(NSArray *)ruleIds filterId:(NSNumber *)filterId;

/**
 Retuns list of filters.
 Gets fresh list of filters.
 Tries load metadata from backend service if need it
 or obtains filters metadata from default DB.
 Request to backend is performed synchronous.
 
 @param refresh Makes attempting to download metadata from the backend forced.
 
 @return Array of ASDFilterMetadata objects or nil if error occurs.
 */
- (NSArray *)filtersForSubscribe:(BOOL)refresh;

/**
 Retuns list of groups.
 Gets fresh list of groups.
 Tries load metadata from backend service if need it
 or obtains groups metadata from default DB.
 Request to backend is performed synchronous.
 
 @param refresh Makes attempting to download metadata from the backend forced.
 
 @return Array of ASDFilterGroup objects or nil if error occurs.
 */
- (NSArray *)groupsForSubscribe:(BOOL)refresh;

/**
 Performs subscription to filters.
 Inserts filter metadata from filters parameter into production DB.
 Copies the rules from default DB into production DB if they present,
 or tries to obtain the rules from backend server.
 
 @param filters         List of the ASDFilterMetadata objects, which will be save into DB.
 @param jobController   Because the subscription process is long,
 you may use this parameter for cancellation.
 */
- (BOOL)subscribeFilters:(NSArray *)filters jobController:(ACLJobController *)jobController;

/**
 Removes filter data from production DB.
 */
- (BOOL)unsubscribeFilter:(NSNumber *)filterId;

/**
 Starts updating of filters from Backend service.
 */
- (void)startUpdating;


- (BOOL)inTransaction;
- (void)beginTransaction;
- (void)endTransaction;
- (void)rollbackTransaction;

@end