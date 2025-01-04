--- ../qtbase-everywhere-src-6.6.3/src/corelib/kernel/qtimerinfo_unix.cpp	2024-03-01 02:23:07.000000000 +1030
+++ src/corelib/kernel/qtimerinfo_unix.cpp.orig	2025-01-04 15:27:45.881959658 +1030
@@ -31,6 +31,10 @@
 QTimerInfoList::QTimerInfoList()
 {
     firstTimerInfo = nullptr;
+
+#ifdef Q_OS_OS2
+    zeroTimers = 0;
+#endif
 }
 
 timespec QTimerInfoList::updateCurrentTime()
@@ -89,10 +93,10 @@
 }
 
 #ifdef QTIMERINFO_DEBUG
-QDebug operator<<(QDebug s, timeval tv)
+QDebug operator<<(QDebug s, timespec tv)
 {
     QDebugStateSaver saver(s);
-    s.nospace() << tv.tv_sec << "." << qSetFieldWidth(6) << qSetPadChar(QChar(48)) << tv.tv_usec << Qt::reset;
+    s.nospace() << tv.tv_sec << "." << qSetFieldWidth(6) << qSetPadChar(QChar(48)) << tv.tv_nsec << Qt::reset;
     return s;
 }
 QDebug operator<<(QDebug s, Qt::TimerType t)
@@ -370,6 +374,13 @@
             ++t->timeout.tv_sec;
     }
 
+#ifdef Q_OS_OS2
+    if (t->interval == 0ms) {
+        ++zeroTimers;
+        Q_ASSERT(zeroTimers);
+    }
+#endif
+
     timerInsert(t);
 
 #ifdef QTIMERINFO_DEBUG
@@ -394,6 +405,12 @@
         firstTimerInfo = nullptr;
     if (t->activateRef)
         *(t->activateRef) = nullptr;
+#ifdef Q_OS_OS2
+     if (t->interval == 0ms) {
+         Q_ASSERT(zeroTimers);
+         --zeroTimers;
+     }
+#endif
     delete t;
     erase(it);
     return true;
@@ -412,6 +429,12 @@
                 firstTimerInfo = nullptr;
             if (t->activateRef)
                 *(t->activateRef) = nullptr;
+#ifdef Q_OS_OS2
+            if (t->interval == 0ms) {
+                Q_ASSERT(zeroTimers);
+                --zeroTimers;
+            }
+#endif
             delete t;
             // move back one so that we don't skip the new current item
             --i;
@@ -475,11 +498,11 @@
         float diff;
         if (currentTime < currentTimerInfo->expected) {
             // early
-            timeval early = currentTimerInfo->expected - currentTime;
-            diff = -(early.tv_sec + early.tv_usec / 1000000.0);
+            timespec early = currentTimerInfo->expected - currentTime;
+            diff = -(early.tv_sec + early.tv_nsec / 1000000000.0);
         } else {
-            timeval late = currentTime - currentTimerInfo->expected;
-            diff = late.tv_sec + late.tv_usec / 1000000.0;
+            timespec late = currentTime - currentTimerInfo->expected;
+            diff = late.tv_sec + late.tv_nsec / 1000000000.0;
         }
         currentTimerInfo->cumulativeError += diff;
         ++currentTimerInfo->count;
