#include <QtTest>
#include "core/CalendarEvent.h"
#include "core/Task.h"
#include "core/CalendarManager.h"

class TestAdapters : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // CalendarEvent 測試
    void testCalendarEventDefault();
    void testCalendarEventValid();
    void testCalendarEventDateRange();
    void testCalendarEventComparison();

    // Task 測試
    void testTaskDefault();
    void testTaskValid();
    void testTaskOverdue();
    void testTaskComparison();

    // CalendarManager 測試
    void testAddEvent();
    void testRemoveEvent();
    void testUpdateEvent();
    void testAddTask();
    void testRemoveTask();
    void testSetTaskCompleted();

private:
    CalendarManager* m_manager;
};

void TestAdapters::initTestCase()
{
    m_manager = new CalendarManager(this);
}

void TestAdapters::cleanupTestCase()
{
    delete m_manager;
}

// CalendarEvent 測試

void TestAdapters::testCalendarEventDefault()
{
    CalendarEvent event;
    QVERIFY(!event.isValid());
    QCOMPARE(event.platform, Platform::Google);
    QCOMPARE(event.isAllDay, false);
}

void TestAdapters::testCalendarEventValid()
{
    CalendarEvent event;
    event.id = "event1";
    event.title = "Test Event";
    event.startTime = QDateTime::currentDateTime();
    
    QVERIFY(event.isValid());
}

void TestAdapters::testCalendarEventDateRange()
{
    CalendarEvent event;
    event.id = "event1";
    event.title = "Test Event";
    event.startTime = QDateTime(QDate(2024, 6, 15), QTime(10, 0));
    event.endTime = QDateTime(QDate(2024, 6, 15), QTime(11, 0));
    
    QVERIFY(event.isInDateRange(QDate(2024, 6, 1), QDate(2024, 6, 30)));
    QVERIFY(!event.isInDateRange(QDate(2024, 7, 1), QDate(2024, 7, 31)));
}

void TestAdapters::testCalendarEventComparison()
{
    CalendarEvent event1;
    event1.id = "event1";
    event1.startTime = QDateTime(QDate(2024, 6, 15), QTime(10, 0));
    
    CalendarEvent event2;
    event2.id = "event2";
    event2.startTime = QDateTime(QDate(2024, 6, 15), QTime(14, 0));
    
    QVERIFY(event1 < event2);
    QVERIFY(!(event1 == event2));
    
    CalendarEvent event3 = event1;
    QVERIFY(event1 == event3);
}

// Task 測試

void TestAdapters::testTaskDefault()
{
    Task task;
    QVERIFY(!task.isValid());
    QCOMPARE(task.platform, Platform::Google);
    QCOMPARE(task.isCompleted, false);
    QCOMPARE(task.priority, 3);
}

void TestAdapters::testTaskValid()
{
    Task task;
    task.id = "task1";
    task.title = "Test Task";
    
    QVERIFY(task.isValid());
}

void TestAdapters::testTaskOverdue()
{
    Task task;
    task.id = "task1";
    task.title = "Overdue Task";
    task.dueDate = QDateTime::currentDateTime().addDays(-1);
    task.isCompleted = false;
    
    QVERIFY(task.isOverdue());
    
    task.isCompleted = true;
    QVERIFY(!task.isOverdue());
}

void TestAdapters::testTaskComparison()
{
    Task task1;
    task1.id = "task1";
    task1.title = "High Priority";
    task1.priority = 1;
    task1.isCompleted = false;
    
    Task task2;
    task2.id = "task2";
    task2.title = "Low Priority";
    task2.priority = 5;
    task2.isCompleted = false;
    
    QVERIFY(task1 < task2); // 高優先級優先
    
    Task task3 = task2;
    task3.isCompleted = true;
    
    QVERIFY(task2 < task3); // 未完成優先
}

// CalendarManager 測試

void TestAdapters::testAddEvent()
{
    CalendarEvent event;
    event.id = "mgr_event1";
    event.title = "Manager Event";
    event.startTime = QDateTime::currentDateTime();
    event.platform = Platform::Google;
    
    QVERIFY(m_manager->addEvent(event));
    QCOMPARE(m_manager->getAllEvents().size(), 1);
    
    // 不能重複添加
    QVERIFY(!m_manager->addEvent(event));
}

void TestAdapters::testRemoveEvent()
{
    int initialCount = m_manager->getAllEvents().size();
    QVERIFY(m_manager->removeEvent("mgr_event1", Platform::Google));
    QCOMPARE(m_manager->getAllEvents().size(), initialCount - 1);
}

void TestAdapters::testUpdateEvent()
{
    CalendarEvent event;
    event.id = "mgr_event2";
    event.title = "Original Title";
    event.startTime = QDateTime::currentDateTime();
    event.platform = Platform::Google;
    
    m_manager->addEvent(event);
    
    event.title = "Updated Title";
    QVERIFY(m_manager->updateEvent(event));
    
    auto events = m_manager->getAllEvents();
    auto it = std::find_if(events.begin(), events.end(), [](const CalendarEvent& e) {
        return e.id == "mgr_event2";
    });
    QVERIFY(it != events.end());
    QCOMPARE(it->title, QString("Updated Title"));
}

void TestAdapters::testAddTask()
{
    Task task;
    task.id = "mgr_task1";
    task.title = "Manager Task";
    task.platform = Platform::Apple;
    
    QVERIFY(m_manager->addTask(task));
    QVERIFY(m_manager->getAllTasks().size() >= 1);
}

void TestAdapters::testRemoveTask()
{
    QVERIFY(m_manager->removeTask("mgr_task1", Platform::Apple));
}

void TestAdapters::testSetTaskCompleted()
{
    Task task;
    task.id = "mgr_task2";
    task.title = "Complete Me";
    task.platform = Platform::Outlook;
    task.isCompleted = false;
    
    m_manager->addTask(task);
    QVERIFY(m_manager->setTaskCompleted("mgr_task2", Platform::Outlook, true));
    
    auto tasks = m_manager->getAllTasks();
    auto it = std::find_if(tasks.begin(), tasks.end(), [](const Task& t) {
        return t.id == "mgr_task2";
    });
    QVERIFY(it != tasks.end());
    QCOMPARE(it->isCompleted, true);
}

QTEST_MAIN(TestAdapters)
#include "test_adapters.moc"
