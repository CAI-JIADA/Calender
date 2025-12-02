#include <QtTest>
#include "core/CalendarEvent.h"
#include "core/Task.h"
#include "core/CalendarManager.h"
#include "core/SearchCriteria.h"
#include "search/SearchEngine.h"

class TestSearch : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // SearchCriteria 測試
    void testSearchCriteriaDefault();
    void testSearchCriteriaDateRange();
    void testSearchCriteriaClear();

    // SearchEngine 測試
    void testSearchEventsEmpty();
    void testSearchEventsKeyword();
    void testSearchEventsByDateRange();
    void testSearchEventsByPlatform();
    void testAdvancedSearch();
    void testSearchTasks();
    void testSearchAll();

private:
    CalendarManager* m_manager;
    SearchEngine* m_searchEngine;
    
    void setupTestData();
};

void TestSearch::initTestCase()
{
    m_manager = new CalendarManager(this);
    m_searchEngine = new SearchEngine(m_manager, this);
    setupTestData();
}

void TestSearch::cleanupTestCase()
{
    delete m_searchEngine;
    delete m_manager;
}

void TestSearch::setupTestData()
{
    // 添加測試事件
    CalendarEvent event1;
    event1.id = "search_event1";
    event1.title = "Team Meeting";
    event1.description = "Weekly team sync";
    event1.location = "Conference Room A";
    event1.startTime = QDateTime(QDate(2024, 6, 15), QTime(10, 0));
    event1.endTime = QDateTime(QDate(2024, 6, 15), QTime(11, 0));
    event1.platform = Platform::Google;
    m_manager->addEvent(event1);

    CalendarEvent event2;
    event2.id = "search_event2";
    event2.title = "Project Review";
    event2.description = "Review project progress";
    event2.location = "Room B";
    event2.startTime = QDateTime(QDate(2024, 6, 20), QTime(14, 0));
    event2.endTime = QDateTime(QDate(2024, 6, 20), QTime(15, 0));
    event2.platform = Platform::Outlook;
    m_manager->addEvent(event2);

    CalendarEvent event3;
    event3.id = "search_event3";
    event3.title = "Lunch with Client";
    event3.description = "Business lunch";
    event3.location = "Restaurant Downtown";
    event3.startTime = QDateTime(QDate(2024, 7, 5), QTime(12, 0));
    event3.endTime = QDateTime(QDate(2024, 7, 5), QTime(13, 30));
    event3.platform = Platform::Apple;
    m_manager->addEvent(event3);

    // 添加測試任務
    Task task1;
    task1.id = "search_task1";
    task1.title = "Complete Report";
    task1.description = "Finish the quarterly report";
    task1.dueDate = QDateTime(QDate(2024, 6, 25), QTime(17, 0));
    task1.platform = Platform::Google;
    task1.isCompleted = false;
    m_manager->addTask(task1);

    Task task2;
    task2.id = "search_task2";
    task2.title = "Review Code";
    task2.description = "Code review for feature branch";
    task2.dueDate = QDateTime(QDate(2024, 6, 18), QTime(12, 0));
    task2.platform = Platform::Outlook;
    task2.isCompleted = true;
    m_manager->addTask(task2);
}

// SearchCriteria 測試

void TestSearch::testSearchCriteriaDefault()
{
    SearchCriteria criteria;
    QVERIFY(criteria.isEmpty());
    QVERIFY(criteria.keyword.isEmpty());
    QVERIFY(!criteria.hasDateRange());
    QVERIFY(!criteria.hasPlatformFilter());
    QCOMPARE(criteria.includeCompleted, false);
}

void TestSearch::testSearchCriteriaDateRange()
{
    SearchCriteria criteria;
    criteria.startDate = QDate(2024, 6, 1);
    criteria.endDate = QDate(2024, 6, 30);
    
    QVERIFY(criteria.hasDateRange());
    QVERIFY(!criteria.isEmpty());
}

void TestSearch::testSearchCriteriaClear()
{
    SearchCriteria criteria("test");
    criteria.startDate = QDate(2024, 6, 1);
    criteria.endDate = QDate(2024, 6, 30);
    criteria.platforms << "Google";
    
    QVERIFY(!criteria.isEmpty());
    
    criteria.clear();
    QVERIFY(criteria.isEmpty());
}

// SearchEngine 測試

void TestSearch::testSearchEventsEmpty()
{
    auto results = m_searchEngine->searchEvents("");
    QVERIFY(results.size() >= 3); // 至少有我們添加的測試事件
}

void TestSearch::testSearchEventsKeyword()
{
    auto results = m_searchEngine->searchEvents("Meeting");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().id, QString("search_event1"));
    
    results = m_searchEngine->searchEvents("review");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().id, QString("search_event2"));
    
    // 搜索地點
    results = m_searchEngine->searchEvents("Conference");
    QCOMPARE(results.size(), 1);
}

void TestSearch::testSearchEventsByDateRange()
{
    auto results = m_searchEngine->searchEventsByDateRange(
        QDate(2024, 6, 1), QDate(2024, 6, 30));
    QCOMPARE(results.size(), 2); // 6月的事件
    
    results = m_searchEngine->searchEventsByDateRange(
        QDate(2024, 7, 1), QDate(2024, 7, 31));
    QCOMPARE(results.size(), 1); // 7月的事件
    
    // 搜索帶關鍵字
    results = m_searchEngine->searchEventsByDateRange(
        QDate(2024, 6, 1), QDate(2024, 6, 30), "Project");
    QCOMPARE(results.size(), 1);
}

void TestSearch::testSearchEventsByPlatform()
{
    QStringList platforms;
    platforms << "Google";
    
    auto results = m_searchEngine->searchEventsByPlatform("", platforms);
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().platform, Platform::Google);
    
    platforms << "Outlook";
    results = m_searchEngine->searchEventsByPlatform("", platforms);
    QCOMPARE(results.size(), 2);
}

void TestSearch::testAdvancedSearch()
{
    SearchCriteria criteria;
    criteria.keyword = "review";
    criteria.platforms << "Outlook";
    
    auto results = m_searchEngine->advancedSearchEvents(criteria);
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().id, QString("search_event2"));
}

void TestSearch::testSearchTasks()
{
    auto results = m_searchEngine->searchTasks("Report");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().id, QString("search_task1"));
    
    results = m_searchEngine->searchTasks("Code");
    QCOMPARE(results.size(), 1);
}

void TestSearch::testSearchAll()
{
    QList<CalendarEvent> events;
    QList<Task> tasks;
    
    m_searchEngine->searchAll("review", events, tasks);
    
    QCOMPARE(events.size(), 1); // "review" in event description
    QCOMPARE(tasks.size(), 1);  // "Review" in task title
}

QTEST_MAIN(TestSearch)
#include "test_search.moc"
